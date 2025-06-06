#ifndef CXX_INDEXER_COMMAND_PROVIDER_H
#define CXX_INDEXER_COMMAND_PROVIDER_H

#include <map>
#include <set>
#include <string>
#include <unordered_map>

#include "IndexerCommandProvider.h"
#include "types.h"

class IndexerCommandCxx;

class CxxIndexerCommandProvider: public IndexerCommandProvider
{
public:
	CxxIndexerCommandProvider();
	void addCommand(const std::shared_ptr<IndexerCommandCxx>& command);
	std::vector<FilePath> getAllSourceFilePaths() const override;
	std::shared_ptr<IndexerCommand> consumeCommand() override;
	std::shared_ptr<IndexerCommand> consumeCommandForSourceFilePath(const FilePath& filePath) override;
	std::vector<std::shared_ptr<IndexerCommand>> consumeAllCommands() override;
	void clear() override;
	size_t size() const override;
	void logStats() const;

private:
	struct CommandRepresentation
	{
		std::set<Id> m_indexedPathIds;
		std::set<Id> m_excludeFilterIds;
		std::set<Id> m_includeFilterIds;
		Id m_workingDirectoryId;
		std::vector<Id> m_compilerFlagIds;
	};

	Id getId();
	std::shared_ptr<IndexerCommandCxx> representationToCommand(
		const FilePath& sourceFilePath, std::shared_ptr<CommandRepresentation> representation);

	Id m_nextId;

	std::multimap<FilePath, std::shared_ptr<CommandRepresentation>> m_commands;

	std::map<Id, FilePath> m_idsToIndexedPaths;
	std::map<FilePath, Id> m_indexedPathsToIds;
	std::map<Id, std::string> m_idsToExcludeFilters;
	std::map<std::string, Id> m_excludeFiltersToIds;
	std::map<Id, std::string> m_idsToIncludeFilters;
	std::map<std::string, Id> m_includeFiltersToIds;
	std::map<Id, FilePath> m_idsToWorkingDirectories;
	std::map<FilePath, Id> m_workingDirectoriesToIds;
	std::map<Id, std::string> m_idsToCompilerFlags;
	std::unordered_map<std::string, Id> m_compilerFlagsToIds;
};

#endif	  // CXX_INDEXER_COMMAND_PROVIDER_H
