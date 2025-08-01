#include "SourceGroupCxxCdb.h"

#include "Application.h"
#include "ApplicationSettings.h"
#include "ClangInvocationInfo.h"
#include "CxxCompilationDatabaseSingle.h"
#include "CxxIndexerCommandProvider.h"
#include "IndexerCommandCxx.h"
#include "MessageStatus.h"
#include "SourceGroupSettingsCxxCdb.h"
#include "TaskLambda.h"
#include "utility.h"
#include "utilitySourceGroupCxx.h"
#include "utilityString.h"

#include <clang/Tooling/JSONCompilationDatabase.h>
#include <clang/Tooling/Tooling.h>

SourceGroupCxxCdb::SourceGroupCxxCdb(std::shared_ptr<SourceGroupSettingsCxxCdb> settings)
	: m_settings(settings)
{
}

bool SourceGroupCxxCdb::prepareIndexing()
{
	FilePath cdbPath = m_settings->getCompilationDatabasePathExpandedAndAbsolute();
	if (!cdbPath.empty() && !cdbPath.exists())
	{
		std::string error =
			"Can't refresh project. The compilation database of the project does not exist "
			"anymore: " +
			cdbPath.str();
		MessageStatus(error, true).dispatch();
		Application::getInstance()->handleDialog(error, {"Ok"});
		return false;
	}
	return true;
}

std::set<FilePath> SourceGroupCxxCdb::filterToContainedFilePaths(const std::set<FilePath>& filePaths) const
{
	return SourceGroup::filterToContainedFilePaths(
		filePaths,
		getAllSourceFilePaths(),
		utility::toSet(m_settings->getIndexedHeaderPathsExpandedAndAbsolute()),
		m_settings->getExcludeFiltersExpandedAndAbsolute());
}

std::set<FilePath> SourceGroupCxxCdb::getAllSourceFilePaths() const
{
	return getAllSourceFilePaths(
		utility::loadCDB(m_settings->getCompilationDatabasePathExpandedAndAbsolute()));
}

std::set<FilePath> SourceGroupCxxCdb::getAllSourceFilePaths(
	std::shared_ptr<clang::tooling::JSONCompilationDatabase> cdb) const
{
	std::set<FilePath> sourceFilePaths;

	if (cdb)
	{
		const std::vector<FilePathFilter> excludeFilters =
			m_settings->getExcludeFiltersExpandedAndAbsolute();
		for (const FilePath& path: IndexerCommandCxx::getSourceFilesFromCDB(
				 cdb, m_settings->getCompilationDatabasePathExpandedAndAbsolute()))
		{
			bool excluded = FilePathFilter::areMatching(excludeFilters, path);
			if (!excluded && path.exists())
			{
				sourceFilePaths.insert(path);
			}
		}
	}

	return sourceFilePaths;
}

std::shared_ptr<IndexerCommandProvider> SourceGroupCxxCdb::getIndexerCommandProvider(const RefreshInfo &info) const
{
	std::shared_ptr<CxxIndexerCommandProvider> provider = std::make_shared<CxxIndexerCommandProvider>();

	const FilePath cdbPath = m_settings->getCompilationDatabasePathExpandedAndAbsolute();
	std::shared_ptr<clang::tooling::JSONCompilationDatabase> cdb = utility::loadCDB(cdbPath);
	if (!cdb)
	{
		return provider;
	}

	std::vector<std::string> compilerFlags = getBaseCompilerFlags();
	utility::append(compilerFlags, m_settings->getCompilerFlags());

	const std::vector<std::string> includePchFlags = utility::getIncludePchFlags(m_settings.get());

	const std::set<FilePath> indexedHeaderPaths = utility::toSet(m_settings->getIndexedHeaderPathsExpandedAndAbsolute());
	const std::set<FilePathFilter> excludeFilters = utility::toSet(m_settings->getExcludeFiltersExpandedAndAbsolute());
	const std::set<FilePath> &sourceFilePaths = getAllSourceFilePaths(cdb);

	for (const clang::tooling::CompileCommand &command : cdb->getAllCompileCommands())
	{
		FilePath sourcePath = FilePath(command.Filename).makeCanonical();
		if (!sourcePath.isAbsolute())
		{
			sourcePath = FilePath(command.Directory + '/' + command.Filename).makeCanonical();
			if (!sourcePath.isAbsolute())
			{
				sourcePath = cdbPath.getParentDirectory().getConcatenated(sourcePath).makeCanonical();
			}
		}

		if (info.filesToIndex.find(sourcePath) != info.filesToIndex.end() && sourceFilePaths.find(sourcePath) != sourceFilePaths.end())
		{
			std::vector<std::string> commandLine = command.CommandLine;
			
			utility::removeIncludePchFlag(commandLine);
			utility::replaceMsvcArguments(&commandLine);

			if (command.CommandLine.size() != commandLine.size())
			{
				utility::append(commandLine, includePchFlags);
			}

			provider->addCommand(std::make_shared<IndexerCommandCxx>(sourcePath, utility::concat(indexedHeaderPaths, {sourcePath}), excludeFilters,
				std::set<FilePathFilter>(), FilePath(command.Directory), utility::concat(commandLine, compilerFlags)));
		}
	}

	provider->logStats();

	return provider;
}

std::vector<std::shared_ptr<IndexerCommand>> SourceGroupCxxCdb::getIndexerCommands(
	const RefreshInfo& info) const
{
	return getIndexerCommandProvider(info)->consumeAllCommands();
}

std::shared_ptr<Task> SourceGroupCxxCdb::getPreIndexTask(
	std::shared_ptr<StorageProvider> storageProvider, std::shared_ptr<DialogView> dialogView) const
{
	if (m_settings->getPchInputFilePath().empty())
	{
		return std::make_shared<TaskLambda>([]() {});
	}

	std::vector<std::string> compilerFlags;

	if (m_settings->getUseCompilerFlags())
	{
		const FilePath cdbPath = m_settings->getCompilationDatabasePathExpandedAndAbsolute();
		std::shared_ptr<clang::tooling::JSONCompilationDatabase> cdb = utility::loadCDB(cdbPath);
		if (cdb)
		{
			const std::set<FilePath> sourceFilePaths = getAllSourceFilePaths(cdb);
			for (const clang::tooling::CompileCommand& command: cdb->getAllCompileCommands())
			{
				FilePath sourcePath = FilePath(command.Filename).makeCanonical();
				if (!sourcePath.isAbsolute())
				{
					sourcePath = FilePath(command.Directory + '/' + command.Filename).makeCanonical();
					if (!sourcePath.isAbsolute())
					{
						sourcePath =
							cdbPath.getParentDirectory().getConcatenated(sourcePath).makeCanonical();
					}
				}

				if (sourceFilePaths.find(sourcePath) != sourceFilePaths.end() &&
					utility::containsIncludePchFlag(command.CommandLine))
				{
					for (const std::string& arg: command.CommandLine)
					{
						if ((!compilerFlags.empty() || utility::isPrefix("-", arg)) &&
							FilePath(arg).fileName() != sourcePath.fileName())
						{
							compilerFlags.emplace_back(arg);
						}
					}

					CxxCompilationDatabaseSingle compilationDatabase(command);
					ClangInvocationInfo info = ClangInvocationInfo::getClangInvocationString(
						&compilationDatabase);

					if (info.invocation.find("\"-x\" \"c++\""))
					{
						compilerFlags.push_back("-x");
						compilerFlags.push_back("c++");
					}
					break;
				}
			}
		}
	}

	utility::append(compilerFlags, getBaseCompilerFlags());

	if (m_settings->getUseCompilerFlags())
	{
		utility::append(compilerFlags, m_settings->getCompilerFlags());
	}

	utility::append(compilerFlags, m_settings->getPchFlags());

	return utility::createBuildPchTask(m_settings.get(), compilerFlags, storageProvider, dialogView);
}

std::shared_ptr<SourceGroupSettings> SourceGroupCxxCdb::getSourceGroupSettings()
{
	return m_settings;
}

std::shared_ptr<const SourceGroupSettings> SourceGroupCxxCdb::getSourceGroupSettings() const
{
	return m_settings;
}

std::vector<std::string> SourceGroupCxxCdb::getBaseCompilerFlags() const
{
	std::vector<std::string> compilerFlags;

	std::shared_ptr<ApplicationSettings> appSettings = ApplicationSettings::getInstance();

	utility::append(
		compilerFlags,
		IndexerCommandCxx::getCompilerFlagsForSystemHeaderSearchPaths(utility::concat(
			m_settings->getHeaderSearchPathsExpandedAndAbsolute(),
			appSettings->getHeaderSearchPathsExpanded())));

	utility::append(
		compilerFlags,
		IndexerCommandCxx::getCompilerFlagsForFrameworkSearchPaths(utility::concat(
			m_settings->getFrameworkSearchPathsExpandedAndAbsolute(),
			appSettings->getFrameworkSearchPathsExpanded())));

	return compilerFlags;
}
