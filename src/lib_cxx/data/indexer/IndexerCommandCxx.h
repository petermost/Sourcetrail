#ifndef INDEXER_COMMAND_CXX_H
#define INDEXER_COMMAND_CXX_H

#include <string>
#include <vector>

#include "IndexerCommand.h"

class FilePath;

namespace clang::tooling
{
class CompilationDatabase;
}

class IndexerCommandCxx: public IndexerCommand
{
public:
	static std::vector<FilePath> getSourceFilesFromCDB(const FilePath& cdbPath);
	static std::vector<FilePath> getSourceFilesFromCDB(std::shared_ptr<clang::tooling::CompilationDatabase> cdb, const FilePath& cdbPath);

	static std::string getCompilerFlagLanguageStandard(const std::string& languageStandard);
	static std::vector<std::string> getCompilerFlagsForSystemHeaderSearchPaths(
		const std::vector<FilePath>& systemHeaderSearchPaths);
	static std::vector<std::string> getCompilerFlagsForFrameworkSearchPaths(
		const std::vector<FilePath>& frameworkSearchPaths);

	static IndexerCommandType getStaticIndexerCommandType();

	IndexerCommandCxx(
		const FilePath& sourceFilePath,
		const std::set<FilePath>& indexedPaths,
		const std::set<FilePathFilter>& excludeFilters,
		const std::set<FilePathFilter>& includeFilters,
		const FilePath& workingDirectory,
		const std::vector<std::string>& compilerFlags);

	IndexerCommandType getIndexerCommandType() const override;
	size_t getByteSize(size_t stringSize) const override;

	const std::set<FilePath>& getIndexedPaths() const;
	const std::set<FilePathFilter>& getExcludeFilters() const;
	const std::set<FilePathFilter>& getIncludeFilters() const;
	const std::vector<std::string>& getCompilerFlags() const;
	const FilePath& getWorkingDirectory() const;

protected:
	QJsonObject doSerialize() const override;

private:
	std::set<FilePath> m_indexedPaths;
	std::set<FilePathFilter> m_excludeFilters;
	std::set<FilePathFilter> m_includeFilters;
	FilePath m_workingDirectory;
	std::vector<std::string> m_compilerFlags;
};

#endif	  // INDEXER_COMMAND_CXXL_H
