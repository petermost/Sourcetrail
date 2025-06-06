#ifndef INDEXER_COMMAND_H
#define INDEXER_COMMAND_H

#include <set>
#include <string>

#include "FilePath.h"
#include "FilePathFilter.h"
#include "IndexerCommandType.h"

class QJsonObject;

class IndexerCommand
{
public:
	static std::string serialize(
		std::shared_ptr<const IndexerCommand> indexerCommand, bool compact = true);

	IndexerCommand(const FilePath& sourceFilePath);
	virtual ~IndexerCommand() = default;

	virtual IndexerCommandType getIndexerCommandType() const = 0;

	virtual size_t getByteSize(size_t stringSize) const;

	const FilePath& getSourceFilePath() const;

protected:
	virtual QJsonObject doSerialize() const;

private:
	FilePath m_sourceFilePath;
};

#endif	  // INDEXER_COMMAND_H
