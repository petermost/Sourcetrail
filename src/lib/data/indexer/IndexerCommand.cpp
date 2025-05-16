#include "IndexerCommand.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "utilityString.h"

std::string IndexerCommand::serialize(std::shared_ptr<const IndexerCommand> indexerCommand, bool compact)
{
	QJsonDocument jsonDocument(indexerCommand->doSerialize());
	return QString::fromUtf8(
			   jsonDocument.toJson(compact ? QJsonDocument::Compact : QJsonDocument::Indented))
		.toStdString();
}

IndexerCommand::IndexerCommand(const FilePath& sourceFilePath): m_sourceFilePath(sourceFilePath) {}

size_t IndexerCommand::getByteSize(size_t  /*stringSize*/) const
{
	return utility::encodeToUtf8(m_sourceFilePath.str()).size();
}

const FilePath& IndexerCommand::getSourceFilePath() const
{
	return m_sourceFilePath;
}

QJsonObject IndexerCommand::doSerialize() const
{
	QJsonObject jsonObject;

	{
		jsonObject["type"] = QString::fromStdString(
			indexerCommandTypeToString(getIndexerCommandType()));
	}
	{
		jsonObject["source_file_path"] = QString::fromStdString(m_sourceFilePath.str());
	}

	return jsonObject;
}
