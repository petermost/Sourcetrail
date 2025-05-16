#ifndef INDEXER_COMMAND_CUSTOM_H
#define INDEXER_COMMAND_CUSTOM_H

#include <string>
#include <vector>

#include "IndexerCommand.h"

class IndexerCommandCustom: public IndexerCommand
{
public:
	static IndexerCommandType getStaticIndexerCommandType();

	IndexerCommandCustom(
		const std::string& command,
		const std::vector<std::string>& arguments,
		const FilePath& projectFilePath,
		const FilePath& databaseFilePath,
		const std::string& databaseVersion,
		const FilePath& sourceFilePath,
		bool runInParallel);

	IndexerCommandCustom(
		IndexerCommandType type,
		const std::string& command,
		const std::vector<std::string>& arguments,
		const FilePath& projectFilePath,
		const FilePath& databaseFilePath,
		const std::string& databaseVersion,
		const FilePath& sourceFilePath,
		bool runInParallel);

	IndexerCommandType getIndexerCommandType() const override;
	size_t getByteSize(size_t stringSize) const override;

	FilePath getDatabaseFilePath() const;
	void setDatabaseFilePath(const FilePath& databaseFilePath);

	std::string getCommand() const;
	std::vector<std::string> getArguments() const;
	bool getRunInParallel() const;

protected:
	QJsonObject doSerialize() const override;

private:
	std::string replaceVariables(std::string s) const;

	IndexerCommandType m_type;
	std::string m_command;
	std::vector<std::string> m_arguments;
	FilePath m_projectFilePath;
	FilePath m_databaseFilePath;
	std::string m_databaseVersion;
	bool m_runInParallel;
};

#endif	  // INDEXER_COMMAND_CXXL_H
