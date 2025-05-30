#include "CompilationDatabase.h"

#include <set>

#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/JSONCompilationDatabase.h>

#include "FilePath.h"
#include "logging.h"
#include "utility.h"
#include "utilityString.h"

utility::CompilationDatabase::CompilationDatabase(const FilePath& filePath): m_filePath(filePath)
{
	init();
}

std::vector<FilePath> utility::CompilationDatabase::getAllHeaderPaths() const
{
	std::vector<FilePath> paths = utility::concat(m_headers, m_systemHeaders);
	paths = utility::unique(paths);
	return paths;
}

std::vector<FilePath> utility::CompilationDatabase::getHeaderPaths() const
{
	return m_headers;
}

std::vector<FilePath> utility::CompilationDatabase::getSystemHeaderPaths() const
{
	return m_systemHeaders;
}

std::vector<FilePath> utility::CompilationDatabase::getFrameworkHeaderPaths() const
{
	return m_frameworkHeaders;
}

void utility::CompilationDatabase::init()
{
	std::string error;
	std::shared_ptr<clang::tooling::JSONCompilationDatabase> cdb(
		clang::tooling::JSONCompilationDatabase::loadFromFile(
			m_filePath.str(),
			error,
			clang::tooling::JSONCommandLineSyntax::AutoDetect));

	if (!cdb)
	{
		LOG_ERROR(
			"Loading compilation database from file \"" + m_filePath.str() +
			"\" failed with error: " + error);
		return;
	}

	std::vector<clang::tooling::CompileCommand> commands = cdb->getAllCompileCommands();
	std::set<FilePath> frameworkHeaders;
	std::set<FilePath> systemHeaders;
	std::set<FilePath> headers;

	{
		const std::string frameworkIncludeFlag = "-iframework";
		const std::string systemIncludeFlag = "-isystem";
		const std::string quoteFlag = "-iquote";
		const std::string includeFlag = "-I";
		for (clang::tooling::CompileCommand& command: commands)
		{
			const std::string commandDirectory = command.Directory;
			for (size_t i = 0; i < command.CommandLine.size(); i++)
			{
				std::string argument = command.CommandLine[i];
				if (i + 1 < command.CommandLine.size() &&
					!utility::isPrefix("-", command.CommandLine[i + 1]))
				{
					argument += command.CommandLine[++i];
				}

				if (utility::isPrefix(frameworkIncludeFlag, argument))
				{
					frameworkHeaders.insert(
						FilePath(
							utility::trim(argument.substr(frameworkIncludeFlag.size())),
							commandDirectory)
							.makeCanonical());
				}
				else if (utility::isPrefix(systemIncludeFlag, argument))
				{
					systemHeaders.insert(FilePath(
											 utility::trim(argument.substr(systemIncludeFlag.size())),
											 commandDirectory)
											 .makeCanonical());
				}
				else if (utility::isPrefix(quoteFlag, argument))
				{
					headers.insert(
						FilePath(utility::trim(argument.substr(quoteFlag.size())), commandDirectory)
							.makeCanonical());
				}
				else if (utility::isPrefix(includeFlag, argument))
				{
					headers.insert(
						FilePath(utility::trim(argument.substr(includeFlag.size())), commandDirectory)
							.makeCanonical());
				}
			}
		}
	}

	m_headers = utility::toVector(headers);
	m_frameworkHeaders = utility::toVector(frameworkHeaders);
	m_systemHeaders = utility::toVector(systemHeaders);
}
