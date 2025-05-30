#include "IndexerCommandCxx.h"

#include "MessageStatus.h"
#include "OrderedCache.h"
#include "Platform.h"
#include "ResourcePaths.h"
#include "logging.h"
#include "utilitySourceGroupCxx.h"

#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/JSONCompilationDatabase.h>

#include <QJsonArray>
#include <QJsonObject>

std::vector<FilePath> IndexerCommandCxx::getSourceFilesFromCDB(const FilePath& cdbPath)
{
	std::string error;
	std::shared_ptr<clang::tooling::JSONCompilationDatabase> cdb = utility::loadCDB(cdbPath, &error);

	if (!error.empty())
	{
		const std::string message = "Loading Clang compilation database failed with error: \"" + error + "\"";
		LOG_ERROR(message);
		MessageStatus(message, true).dispatch();
	}

	return getSourceFilesFromCDB(cdb, cdbPath);
}

std::vector<FilePath> IndexerCommandCxx::getSourceFilesFromCDB(
	std::shared_ptr<clang::tooling::JSONCompilationDatabase> cdb, const FilePath& cdbPath)
{
	std::vector<FilePath> filePaths;
	if (cdb)
	{
		OrderedCache<FilePath, FilePath> canonicalDirectoryPathCache(
			[](const FilePath& path) { return path.getCanonical(); });

		for (const std::string& fileString: cdb->getAllFiles())
		{
			FilePath path = FilePath(fileString);
			if (!path.isAbsolute())
			{
				std::vector<clang::tooling::CompileCommand> commands = cdb->getCompileCommands(fileString);
				if (!commands.empty())
				{
					path = FilePath(commands.front().Directory + '/' + commands.front().Filename).makeCanonical();
				}
			}
			if (!path.isAbsolute())
			{
				path = cdbPath.getParentDirectory().getConcatenated(path).makeCanonical();
			}
			filePaths.push_back(canonicalDirectoryPathCache.getValue(path.getParentDirectory())
									.concatenate(path.fileName()));
		}
	}
	return filePaths;
}

std::string IndexerCommandCxx::getCompilerFlagLanguageStandard(const std::string& languageStandard)
{
	return "-std=" + languageStandard;
}

std::vector<std::string> IndexerCommandCxx::getCompilerFlagsForSystemHeaderSearchPaths(
	const std::vector<FilePath>& systemHeaderSearchPaths)
{
	std::vector<std::string> compilerFlags;
	compilerFlags.reserve(systemHeaderSearchPaths.size() * 2);

	// *Prepend* clang system includes on windows:
	if constexpr (utility::Platform::isWindows()) {
		compilerFlags.push_back("-isystem");
		compilerFlags.push_back(ResourcePaths::getCxxCompilerHeaderDirectoryPath().str());
	}

	for (const FilePath& path: systemHeaderSearchPaths)
	{
		compilerFlags.push_back("-isystem");
		compilerFlags.push_back(path.str());
	}

	// *Append* clang system includes on non-windows:
	if constexpr (!utility::Platform::isWindows()) {
		compilerFlags.push_back("-isystem");
		compilerFlags.push_back(ResourcePaths::getCxxCompilerHeaderDirectoryPath().str());
	}
	return compilerFlags;
}

std::vector<std::string> IndexerCommandCxx::getCompilerFlagsForFrameworkSearchPaths(
	const std::vector<FilePath>& frameworkSearchPaths)
{
	std::vector<std::string> compilerFlags;
	compilerFlags.reserve(frameworkSearchPaths.size() * 2);
	for (const FilePath& path: frameworkSearchPaths)
	{
		compilerFlags.push_back("-iframework");
		compilerFlags.push_back(path.str());
	}
	return compilerFlags;
}

IndexerCommandType IndexerCommandCxx::getStaticIndexerCommandType()
{
	return INDEXER_COMMAND_CXX;
}
IndexerCommandCxx::IndexerCommandCxx(
	const FilePath& sourceFilePath,
	const std::set<FilePath>& indexedPaths,
	const std::set<FilePathFilter>& excludeFilters,
	const std::set<FilePathFilter>& includeFilters,
	const FilePath& workingDirectory,
	const std::vector<std::string>& compilerFlags)
	: IndexerCommand(sourceFilePath)
	, m_indexedPaths(indexedPaths)
	, m_excludeFilters(excludeFilters)
	, m_includeFilters(includeFilters)
	, m_workingDirectory(workingDirectory)
	, m_compilerFlags(compilerFlags)
{
}

IndexerCommandType IndexerCommandCxx::getIndexerCommandType() const
{
	return getStaticIndexerCommandType();
}

size_t IndexerCommandCxx::getByteSize(size_t stringSize) const
{
	size_t size = IndexerCommand::getByteSize(stringSize);

	for (const FilePath& path: m_indexedPaths)
	{
		size += stringSize + path.str().size();
	}

	for (const FilePathFilter& filter: m_excludeFilters)
	{
		size += stringSize + filter.str().size();
	}

	for (const FilePathFilter& filter: m_includeFilters)
	{
		size += stringSize + filter.str().size();
	}

	for (const std::string& flag: m_compilerFlags)
	{
		size += stringSize + flag.size();
	}

	return size;
}

const std::set<FilePath>& IndexerCommandCxx::getIndexedPaths() const
{
	return m_indexedPaths;
}

const std::set<FilePathFilter>& IndexerCommandCxx::getExcludeFilters() const
{
	return m_excludeFilters;
}

const std::set<FilePathFilter>& IndexerCommandCxx::getIncludeFilters() const
{
	return m_includeFilters;
}

const std::vector<std::string>& IndexerCommandCxx::getCompilerFlags() const
{
	return m_compilerFlags;
}

const FilePath& IndexerCommandCxx::getWorkingDirectory() const
{
	return m_workingDirectory;
}

QJsonObject IndexerCommandCxx::doSerialize() const
{
	QJsonObject jsonObject = IndexerCommand::doSerialize();

	{
		QJsonArray indexedPathsArray;
		for (const FilePath& indexedPath: m_indexedPaths)
		{
			indexedPathsArray.append(QString::fromStdString(indexedPath.str()));
		}
		jsonObject["indexed_paths"] = indexedPathsArray;
	}
	{
		QJsonArray excludeFiltersArray;
		for (const FilePathFilter& excludeFilter: m_excludeFilters)
		{
			excludeFiltersArray.append(QString::fromStdString(excludeFilter.str()));
		}
		jsonObject["exclude_filters"] = excludeFiltersArray;
	}
	{
		QJsonArray includeFiltersArray;
		for (const FilePathFilter& includeFilter: m_includeFilters)
		{
			includeFiltersArray.append(QString::fromStdString(includeFilter.str()));
		}
		jsonObject["include_filters"] = includeFiltersArray;
	}
	{
		jsonObject["working_directory"] = QString::fromStdString(getWorkingDirectory().str());
	}
	{
		QJsonArray compilerFlagsArray;
		for (const std::string& compilerFlag: m_compilerFlags)
		{
			compilerFlagsArray.append(QString::fromStdString(compilerFlag));
		}
		jsonObject["compiler_flags"] = compilerFlagsArray;
	}

	return jsonObject;
}
