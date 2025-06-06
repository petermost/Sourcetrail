#include "SqliteIndexStorage.h"

#include "FileSystem.h"
#include "LocationType.h"
#include "SourceLocationCollection.h"
#include "SourceLocationFile.h"
#include "TextAccess.h"
#include "logging.h"
#include "utilityString.h"

const size_t SqliteIndexStorage::s_storageVersion = 25;

namespace
{
std::pair<std::string, std::string> splitLocalSymbolName(const std::string& name)
{
	size_t pos = name.find_last_of('<');
	if (pos == std::string::npos || name.back() != '>')
	{
		return std::make_pair("", "");
	}

	return std::make_pair(name.substr(0, pos), name.substr(pos + 1, name.size() - pos - 2));
}
}	 // namespace

size_t SqliteIndexStorage::getStorageVersion()
{
	return s_storageVersion;
}

SqliteIndexStorage::SqliteIndexStorage(const FilePath& dbFilePath)
	: SqliteStorage(dbFilePath.getCanonical())
{
}

size_t SqliteIndexStorage::getStaticVersion() const
{
	return s_storageVersion;
}

void SqliteIndexStorage::setMode(const StorageModeType mode)
{
	m_tempNodeNameIndex.clear();
	m_tempWNodeNameIndex.clear();
	m_tempNodeTypes.clear();
	m_tempEdgeIndex.clear();
	m_tempLocalSymbolIndex.clear();
	m_tempSourceLocationIndices.clear();

	std::vector<std::pair<int, SqliteDatabaseIndex>> indices = getIndices();
	for (size_t i = 0; i < indices.size(); i++)
	{
		if (indices[i].first & mode)
		{
			indices[i].second.createOnDatabase(m_database);
		}
		else
		{
			indices[i].second.removeFromDatabase(m_database);
		}
	}
}

std::string SqliteIndexStorage::getProjectSettingsText() const
{
	return getMetaValue("project_settings");
}

void SqliteIndexStorage::setProjectSettingsText(std::string text)
{
	insertOrUpdateMetaValue("project_settings", text);
}

Id SqliteIndexStorage::addNode(const StorageNodeData& data)
{
	std::vector<Id> ids = addNodes({StorageNode(0, data)});
	return ids.size() ? ids[0] : 0;
}

std::vector<Id> SqliteIndexStorage::addNodes(const std::vector<StorageNode>& nodes)
{
	if (m_tempNodeNameIndex.empty() && m_tempWNodeNameIndex.empty())
	{
		forEach<StorageNode>([this](StorageNode&& node) {
			std::string name = node.serializedName;
			if (name.size() != node.serializedName.size())
			{
				m_tempWNodeNameIndex.add(node.serializedName, node.id);
			}
			else
			{
				m_tempNodeNameIndex.add(name, node.id);
			}

			m_tempNodeTypes.emplace(node.id, node.type);
		});
	}

	std::vector<Id> nodeIds(nodes.size(), 0);
	std::vector<StorageNode> nodesToInsert;
	for (size_t i = 0; i < nodes.size(); i++)
	{
		const StorageNodeData& data = nodes[i];
		std::string name = data.serializedName;
		{
			Id nodeId;
			if (name.size() != data.serializedName.size())
			{
				nodeId = m_tempWNodeNameIndex.find(data.serializedName);
			}
			else
			{
				nodeId = m_tempNodeNameIndex.find(name);
			}

			if (nodeId)
			{
				auto it = m_tempNodeTypes.find(nodeId);
				if (it != m_tempNodeTypes.end() && it->second < data.type)
				{
					setNodeType(data.type, nodeId);
					m_tempNodeTypes[nodeId] = data.type;
				}

				nodeIds[i] = nodeId;
			}
			else
			{
				executeStatement(m_insertElementStmt);
				const Id id = static_cast<Id>(m_database.lastRowId());

				nodesToInsert.emplace_back(id, data);
				nodeIds[i] = id;

				if (name.size() != data.serializedName.size())
				{
					m_tempWNodeNameIndex.add(data.serializedName, id);
				}
				else
				{
					m_tempNodeNameIndex.add(name, id);
				}
				m_tempNodeTypes.emplace(id, data.type);
			}
		}
	}

	if (nodesToInsert.size())
	{
		m_insertNodeBatchStatement.execute(nodesToInsert, this);
	}

	return nodeIds;
}

bool SqliteIndexStorage::addSymbol(const StorageSymbol& data)
{
	return addSymbols({data});
}

bool SqliteIndexStorage::addSymbols(const std::vector<StorageSymbol>& symbols)
{
	return m_insertSymbolBatchStatement.execute(symbols, this);
}

bool SqliteIndexStorage::addFile(const StorageFile& data)
{
	if (getFileByPath(data.filePath).id != 0)
	{
		return false;
	}

	FilePath filePath(data.filePath);

	std::string modificationTime(data.modificationTime);
	if (modificationTime.empty())
	{
		modificationTime = FileSystem::getFileInfoForPath(filePath).lastWriteTime.toString();
	}

	std::shared_ptr<TextAccess> content;
	int lineCount = 0;
	if (data.indexed)
	{
		content = TextAccess::createFromFile(filePath);
		lineCount = content->getLineCount();
	}

	bool success = false;
	{
		m_insertFileStmt.bind(1, static_cast<Id::type>(data.id));
		m_insertFileStmt.bind(2, data.filePath);
		m_insertFileStmt.bind(3, data.languageIdentifier);
		m_insertFileStmt.bind(4, modificationTime);
		m_insertFileStmt.bind(5, data.indexed);
		m_insertFileStmt.bind(6, data.complete);
		m_insertFileStmt.bind(7, lineCount);
		success = executeStatement(m_insertFileStmt);
	}

	if (success && content)
	{
		m_insertFileContentStmt.bind(1, static_cast<Id::type>(data.id));
		m_insertFileContentStmt.bind(2, content->getText());
		success = executeStatement(m_insertFileContentStmt);
	}

	return success;
}

Id SqliteIndexStorage::addEdge(const StorageEdgeData& data)
{
	std::vector<Id> ids = addEdges({StorageEdge(0, data)});
	return ids.size() ? ids[0] : 0;
}

std::vector<Id> SqliteIndexStorage::addEdges(const std::vector<StorageEdge>& edges)
{
	if (m_tempEdgeIndex.empty())
	{
		forEach<StorageEdge>([this](StorageEdge&& edge) {
			m_tempEdgeIndex.emplace(
				StorageEdgeData(edge.type, edge.sourceNodeId, edge.targetNodeId),
				edge.id);
		});
	}

	std::vector<Id> edgeIds(edges.size(), 0);
	std::vector<StorageEdge> edgesToInsert;
	for (size_t i = 0; i < edges.size(); i++)
	{
		const StorageEdge& data = edges[i];
		const auto it = m_tempEdgeIndex.find(data);
		if (it != m_tempEdgeIndex.end())
		{
			edgeIds[i] = it->second;
		}
		else
		{
			executeStatement(m_insertElementStmt);
			const Id id = static_cast<Id>(m_database.lastRowId());

			edgeIds[i] = id;
			edgesToInsert.emplace_back(id, data);

			m_tempEdgeIndex.emplace(data, id);
		}
	}

	if (edgesToInsert.size())
	{
		m_insertEdgeBatchStatement.execute(edgesToInsert, this);
	}

	return edgeIds;
}

Id SqliteIndexStorage::addLocalSymbol(const StorageLocalSymbolData& data)
{
	std::vector<Id> ids = addLocalSymbols({StorageLocalSymbol(0, data)});
	return ids.size() ? ids[0] : 0;
}

std::vector<Id> SqliteIndexStorage::addLocalSymbols(const std::set<StorageLocalSymbol>& symbols)
{
	if (m_tempLocalSymbolIndex.empty())
	{
		forEach<StorageLocalSymbol>([this](StorageLocalSymbol&& localSymbol) {
			std::pair<std::string, std::string> name = splitLocalSymbolName(localSymbol.name);
			if (name.second.size())
			{
				m_tempLocalSymbolIndex[name.first].emplace(
					name.second, localSymbol.id);
			}
		});
	}

	std::vector<Id> symbolIds(symbols.size(), 0);
	std::vector<StorageLocalSymbol> symbolsToInsert;
	auto it = symbols.begin();
	for (size_t i = 0; i < symbols.size(); i++)
	{
		const StorageLocalSymbol& data = *it;
		std::pair<std::string, std::string> name = splitLocalSymbolName(data.name);
		if (name.second.size())
		{
			auto it = m_tempLocalSymbolIndex.find(name.first);
			if (it != m_tempLocalSymbolIndex.end())
			{
				auto it2 = it->second.find(name.second);
				if (it2 != it->second.end())
				{
					symbolIds[i] = it2->second;
				}
			}
		}

		if (!symbolIds[i])
		{
			executeStatement(m_insertElementStmt);
			const Id id = static_cast<Id>(m_database.lastRowId());

			symbolIds[i] = id;
			symbolsToInsert.emplace_back(id, data);
			if (name.second.size())
			{
				m_tempLocalSymbolIndex[name.first].emplace(name.second, id);
			}
		}

		it++;
	}

	if (symbolsToInsert.size())
	{
		m_insertLocalSymbolBatchStatement.execute(symbolsToInsert, this);
	}

	return symbolIds;
}

Id SqliteIndexStorage::addSourceLocation(const StorageSourceLocationData& data)
{
	std::vector<Id> ids = addSourceLocations({StorageSourceLocation(0, data)});
	return ids.size() ? ids[0] : 0;
}

std::vector<Id> SqliteIndexStorage::addSourceLocations(const std::vector<StorageSourceLocation>& locations)
{
	if (m_tempSourceLocationIndices.empty())
	{
		forEach<StorageSourceLocation>([this](StorageSourceLocation&& loc) {
			auto &index = m_tempSourceLocationIndices[loc.fileNodeId];
			index.emplace(
				TempSourceLocation(
					static_cast<uint32_t>(loc.startLine),
					static_cast<uint16_t>(loc.endLine - loc.startLine),
					static_cast<uint16_t>(loc.startCol),
					static_cast<uint16_t>(loc.endCol),
					static_cast<uint8_t>(loc.type)),
				loc.id);
		});
	}

	std::vector<Id> locationIds(locations.size(), 0);
	std::vector<StorageSourceLocationData> locationsToInsert;
	size_t lastRowId = executeStatementScalar("SELECT MAX(rowid) from source_location");

	for (size_t i = 0; i < locations.size(); i++)
	{
		const StorageSourceLocation& data = locations[i];
		const TempSourceLocation tempLoc(
			static_cast<uint32_t>(data.startLine),
			static_cast<uint16_t>(data.endLine - data.startLine),
			static_cast<uint16_t>(data.startCol),
			static_cast<uint16_t>(data.endCol),
			static_cast<uint8_t>(data.type));

		auto &index = m_tempSourceLocationIndices[data.fileNodeId];
		const auto it = index.find(tempLoc);
		if (it != index.end())
		{
			locationIds[i] = it->second;
		}
		else
		{
			executeStatement(m_insertElementStmt);
			Id id = lastRowId + 1 + locationsToInsert.size();

			locationIds[i] = id;
			index.emplace(tempLoc, id);

			locationsToInsert.emplace_back(data);
		}
	}

	if (locationsToInsert.size())
	{
		m_insertSourceLocationBatchStatement.execute(locationsToInsert, this);
	}

	return locationIds;
}

bool SqliteIndexStorage::addOccurrence(const StorageOccurrence& data)
{
	return addOccurrences({data});
}

bool SqliteIndexStorage::addOccurrences(const std::vector<StorageOccurrence>& occurrences)
{
	return m_insertOccurrenceBatchStatement.execute(occurrences, this);
}

bool SqliteIndexStorage::addComponentAccess(const StorageComponentAccess& componentAccess)
{
	return addComponentAccesses({componentAccess});
}

bool SqliteIndexStorage::addComponentAccesses(const std::vector<StorageComponentAccess>& componentAccesses)
{
	return m_insertComponentAccessBatchStatement.execute(componentAccesses, this);
}

void SqliteIndexStorage::addElementComponent(const StorageElementComponent& component)
{
	m_insertElementComponentStmt.bind(1, static_cast<Id::type>(component.elementId));
	m_insertElementComponentStmt.bind(2, component.type);
	m_insertElementComponentStmt.bind(3, component.data);
	executeStatement(m_insertElementComponentStmt);
	m_insertElementComponentStmt.reset();
}

void SqliteIndexStorage::addElementComponents(const std::vector<StorageElementComponent>& components)
{
	for (const StorageElementComponent& component: components)
	{
		addElementComponent(component);
	}
}

StorageError SqliteIndexStorage::addError(const StorageErrorData& data)
{
	const std::string sanitizedMessage = utility::replace(data.message, "'", "''");

	Id id = 0;
	{
		m_checkErrorExistsStmt.bind(1, sanitizedMessage);
		m_checkErrorExistsStmt.bind(2, int(data.fatal));

		CppSQLite3Query checkQuery = executeQuery(m_checkErrorExistsStmt);
		if (!checkQuery.eof() && checkQuery.numFields() > 0)
		{
			id = checkQuery.getIntField(0, -1);
		}
		m_checkErrorExistsStmt.reset();
	}

	if (id == 0)
	{
		executeStatement(m_insertElementStmt);
		id = static_cast<Id>(m_database.lastRowId());

		m_insertErrorStmt.bind(1, static_cast<Id::type>(id));
		m_insertErrorStmt.bind(2, sanitizedMessage);
		m_insertErrorStmt.bind(3, data.fatal);
		m_insertErrorStmt.bind(4, data.indexed);
		m_insertErrorStmt.bind(5, data.translationUnit);

		const bool success = executeStatement(m_insertErrorStmt);
		if (success)
		{
			id = static_cast<Id>(m_database.lastRowId());
		}
	}

	return StorageError(id, data);
}

void SqliteIndexStorage::removeElement(Id id)
{
	std::vector<Id> ids;
	ids.push_back(id);
	removeElements(ids);
}

void SqliteIndexStorage::removeElements(const std::vector<Id>& ids)
{
	executeStatement(
		"DELETE FROM element WHERE id IN (" + utility::join(utility::toStrings(ids), ',') + ");");
}

void SqliteIndexStorage::removeOccurrence(const StorageOccurrence& occurrence)
{
	executeStatement(
		"DELETE FROM occurrence WHERE element_id = " + to_string(occurrence.elementId) +
		" AND source_location_id = " + to_string(occurrence.sourceLocationId) + ";");
}

void SqliteIndexStorage::removeOccurrences(const std::vector<StorageOccurrence>& occurrences)
{
	for (const StorageOccurrence& occurrence: occurrences)
	{
		removeOccurrence(occurrence);
	}
}

void SqliteIndexStorage::removeElementsWithoutOccurrences(const std::vector<Id>& elementIds)
{
	executeStatement(
		"DELETE FROM element WHERE id IN (" + utility::join(utility::toStrings(elementIds), ',') +
		") AND id NOT IN (SELECT element_id FROM occurrence);");
}

void SqliteIndexStorage::removeElementsWithLocationInFiles(
	const std::vector<Id>& fileIds, std::function<void(int)> updateStatusCallback)
{
	if (updateStatusCallback != nullptr)
	{
		updateStatusCallback(1);
	}

	// preparing
	executeStatement("DROP TABLE IF EXISTS main.element_id_to_clear;");

	if (updateStatusCallback != nullptr)
	{
		updateStatusCallback(2);
	}

	executeStatement(
		"CREATE TABLE IF NOT EXISTS element_id_to_clear("
		"id INTEGER NOT NULL, "
		"PRIMARY KEY(id));");

	if (updateStatusCallback != nullptr)
	{
		updateStatusCallback(3);
	}

	// store ids of all elements located in fileIds into element_id_to_clear
	executeStatement(
		"INSERT INTO element_id_to_clear "
		"	SELECT occurrence.element_id "
		"	FROM occurrence "
		"	INNER JOIN source_location ON ("
		"		occurrence.source_location_id = source_location.id"
		"	) "
		"	WHERE source_location.file_node_id IN (" +
		utility::join(utility::toStrings(fileIds), ',') +
		")"
		"	GROUP BY (occurrence.element_id)");

	if (updateStatusCallback != nullptr)
	{
		updateStatusCallback(4);
	}

	// delete all edges in element_id_to_clear
	executeStatement(
		"DELETE FROM element WHERE element.id IN "
		"	(SELECT element_id_to_clear.id FROM element_id_to_clear INNER JOIN edge ON "
		"(element_id_to_clear.id = edge.id))");

	if (updateStatusCallback != nullptr)
	{
		updateStatusCallback(30);
	}

	// delete all edges originating from element_id_to_clear
	executeStatement(
		"DELETE FROM element WHERE element.id IN (SELECT id FROM edge WHERE source_node_id IN "
		"(SELECT id FROM element_id_to_clear))");

	if (updateStatusCallback != nullptr)
	{
		updateStatusCallback(31);
	}

	// remove all non existing ids from element_id_to_clear (they have been cleared by now and we
	// can disregard them)
	executeStatement(
		"DELETE FROM element_id_to_clear WHERE id NOT IN ("
		"	SELECT id FROM element"
		")");

	if (updateStatusCallback != nullptr)
	{
		updateStatusCallback(33);
	}

	// remove all files from element_id_to_clear (they will be cleared later)
	executeStatement(
		"DELETE FROM element_id_to_clear WHERE id IN ("
		"	SELECT id FROM file"
		")");

	if (updateStatusCallback != nullptr)
	{
		updateStatusCallback(34);
	}

	// delete source locations from fileIds (this also deletes the respective occurrences)
	executeStatement(
		"DELETE FROM source_location WHERE file_node_id IN (" +
		utility::join(utility::toStrings(fileIds), ',') + ");");

	if (updateStatusCallback != nullptr)
	{
		updateStatusCallback(45);
	}

	// remove all ids from element_id_to_clear that still have occurrences
	executeStatement(
		"DELETE FROM element_id_to_clear WHERE id IN ("
		"	SELECT element_id_to_clear.id FROM element_id_to_clear INNER JOIN occurrence ON "
		"		element_id_to_clear.id = occurrence.element_id"
		")");

	if (updateStatusCallback != nullptr)
	{
		updateStatusCallback(59);
	}

	// remove all ids from element_id_to_clear that still have an edge pointing to them
	executeStatement(
		"DELETE FROM element_id_to_clear WHERE id IN ("
		"	SELECT target_node_id FROM edge"
		")");

	if (updateStatusCallback != nullptr)
	{
		updateStatusCallback(74);
	}

	// delete all elements that are still listed in element_id_to_clear
	executeStatement(
		"DELETE FROM element WHERE EXISTS ("
		"	SELECT * FROM element_id_to_clear WHERE element.id = element_id_to_clear.id"
		")");

	if (updateStatusCallback != nullptr)
	{
		updateStatusCallback(87);
	}

	// cleaning up
	executeStatement("DROP TABLE IF EXISTS main.element_id_to_clear;");

	if (updateStatusCallback != nullptr)
	{
		updateStatusCallback(89);
	}
}

void SqliteIndexStorage::removeAllErrors()
{
	executeStatement("DELETE FROM error;");
}

bool SqliteIndexStorage::isEdge(Id elementId) const
{
	int count = executeStatementScalar("SELECT count(*) FROM edge WHERE id = " + to_string(elementId) + ";");
	return (count > 0);
}

bool SqliteIndexStorage::isNode(Id elementId) const
{
	int count = executeStatementScalar("SELECT count(*) FROM node WHERE id = " + to_string(elementId) + ";");
	return (count > 0);
}

bool SqliteIndexStorage::isFile(Id elementId) const
{
	int count = executeStatementScalar("SELECT count(*) FROM file WHERE id = " + to_string(elementId) + ";");
	return (count > 0);
}

StorageEdge SqliteIndexStorage::getEdgeById(Id edgeId) const
{
	std::vector<StorageEdge> candidates = doGetAll<StorageEdge>(
		"WHERE id = " + to_string(edgeId));

	if (candidates.size() > 0)
	{
		return candidates[0];
	}

	return StorageEdge();
}

StorageEdge SqliteIndexStorage::getEdgeBySourceTargetType(Id sourceId, Id targetId, int type) const
{
	return doGetFirst<StorageEdge>(
		"WHERE "
		"source_node_id == " +
		to_string(sourceId) +
		" AND "
		"target_node_id == " +
		to_string(targetId) +
		" AND "
		"type == " +
		std::to_string(type));
}

std::vector<StorageEdge> SqliteIndexStorage::getEdgesBySourceId(Id sourceId) const
{
	return doGetAll<StorageEdge>("WHERE source_node_id == " + to_string(sourceId));
}

std::vector<StorageEdge> SqliteIndexStorage::getEdgesBySourceIds(const std::vector<Id>& sourceIds) const
{
	return doGetAll<StorageEdge>(
		"WHERE source_node_id IN (" + utility::join(utility::toStrings(sourceIds), ',') + ")");
}

std::vector<StorageEdge> SqliteIndexStorage::getEdgesByTargetId(Id targetId) const
{
	return doGetAll<StorageEdge>("WHERE target_node_id == " + to_string(targetId));
}

std::vector<StorageEdge> SqliteIndexStorage::getEdgesByTargetIds(const std::vector<Id>& targetIds) const
{
	return doGetAll<StorageEdge>(
		"WHERE target_node_id IN (" + utility::join(utility::toStrings(targetIds), ',') + ")");
}

std::vector<StorageEdge> SqliteIndexStorage::getEdgesBySourceOrTargetId(Id id) const
{
	return doGetAll<StorageEdge>(
		"WHERE source_node_id == " + to_string(id) +
		" OR target_node_id == " + to_string(id));
}

std::vector<StorageEdge> SqliteIndexStorage::getEdgesByType(int type) const
{
	return doGetAll<StorageEdge>("WHERE type == " + std::to_string(type));
}

std::vector<StorageEdge> SqliteIndexStorage::getEdgesBySourceType(Id sourceId, int type) const
{
	return doGetAll<StorageEdge>(
		"WHERE source_node_id == " + to_string(sourceId) +
		" AND type == " + std::to_string(type));
}

std::vector<StorageEdge> SqliteIndexStorage::getEdgesBySourcesType(
	const std::vector<Id>& sourceIds, int type) const
{
	return doGetAll<StorageEdge>(
		"WHERE source_node_id IN (" + utility::join(utility::toStrings(sourceIds), ',') +
		")"
		" AND type == " +
		std::to_string(type));
}

std::vector<StorageEdge> SqliteIndexStorage::getEdgesByTargetType(Id targetId, int type) const
{
	return doGetAll<StorageEdge>(
		"WHERE target_node_id == " + to_string(targetId) +
		" AND type == " + std::to_string(type));
}

std::vector<StorageEdge> SqliteIndexStorage::getEdgesByTargetsType(
	const std::vector<Id>& targetIds, int type) const
{
	return doGetAll<StorageEdge>(
		"WHERE target_node_id IN (" + utility::join(utility::toStrings(targetIds), ',') +
		")"
		" AND type == " +
		std::to_string(type));
}

StorageNode SqliteIndexStorage::getNodeById(Id id) const
{
	std::vector<StorageNode> candidates = doGetAll<StorageNode>("WHERE id = " + to_string(id));

	if (candidates.size() > 0)
	{
		return candidates[0];
	}

	return StorageNode();
}

StorageNode SqliteIndexStorage::getNodeBySerializedName(const std::string& serializedName) const
{
	CppSQLite3Statement stmt = m_database.compileStatement(
		"SELECT id, type, serialized_name FROM node WHERE serialized_name == ? LIMIT 1;");

	stmt.bind(1, serializedName);
	CppSQLite3Query q = executeQuery(stmt);

	if (!q.eof())
	{
		const Id id = q.getIntField(0, 0);
		const auto type = q.getIntField(1, -1);
		const std::string name = q.getStringField(2, "");

		if (id != 0 && type != -1)
		{
			return StorageNode(id, intToEnum<NodeKind>(type), name);
		}
	}

	stmt.reset();

	return StorageNode();
}

std::vector<NodeKind> SqliteIndexStorage::getAvailableNodeTypes() const
{
	CppSQLite3Query q = executeQuery("SELECT DISTINCT type FROM node;");

	std::vector<NodeKind> types;

	while (!q.eof())
	{
		const auto type = q.getIntField(0, -1);
		if (type != -1)
		{
			types.push_back(intToEnum<NodeKind>(type));
		}

		q.nextRow();
	}

	return types;
}

std::vector<Edge::EdgeType> SqliteIndexStorage::getAvailableEdgeTypes() const
{
	CppSQLite3Query q = executeQuery("SELECT DISTINCT type FROM edge;");

	std::vector<Edge::EdgeType> types;

	while (!q.eof())
	{
		const auto type = q.getIntField(0, -1);
		if (type != -1)
		{
			types.push_back(intToEnum<Edge::EdgeType>(type));
		}

		q.nextRow();
	}

	return types;
}

StorageFile SqliteIndexStorage::getFileByPath(const std::string& filePath) const
{
	return doGetFirst<StorageFile>("WHERE file.path == '" + filePath + "'");
}

std::vector<StorageFile> SqliteIndexStorage::getFilesByPaths(const std::vector<FilePath>& filePaths) const
{
	return doGetAll<StorageFile>(
		"WHERE file.path IN ('" + utility::join(utility::toStrings(filePaths), "', '") + "')");
}

std::shared_ptr<TextAccess> SqliteIndexStorage::getFileContentById(Id fileId) const
{
	CppSQLite3Query q = executeQuery(
		"SELECT content FROM filecontent WHERE id = '" + to_string(fileId) + "';");
	if (!q.eof())
	{
		return TextAccess::createFromString(q.getStringField(0, ""));
	}

	return TextAccess::createFromString("");
}

std::shared_ptr<TextAccess> SqliteIndexStorage::getFileContentByPath(const std::string& filePath) const
{
	try
	{
		CppSQLite3Query q = executeQuery(
			"SELECT filecontent.content "
			"FROM filecontent "
			"INNER JOIN file ON filecontent.id = file.id "
			"WHERE file.path = '" + filePath + "';");

		if (!q.eof())
		{
			return TextAccess::createFromString(q.getStringField(0, ""));
		}
	}
	catch (CppSQLite3Exception& e)
	{
		LOG_ERROR(std::to_string(e.errorCode()) + ": " + e.errorMessage());
	}

	return TextAccess::createFromString("");
}

void SqliteIndexStorage::setFileIndexed(Id fileId, bool indexed)
{
	executeStatement(
		"UPDATE file SET indexed = " + std::to_string(indexed) +
		" WHERE id == " + to_string(fileId) + ";");
}

void SqliteIndexStorage::setFileCompleteIfNoError(Id fileId, const std::string&  /*filePath*/, bool complete)
{
	bool fileHasErrors = doGetFirst<StorageSourceLocation>("WHERE file_node_id == " + to_string(fileId) +
		" AND type == " + to_string(LocationType::ERROR)).id != 0;
	if (fileHasErrors != complete)
	{
		executeStatement(
			"UPDATE file SET complete = " + std::to_string(complete) +
			" WHERE id == " + to_string(fileId) + ";");
	}
}

void SqliteIndexStorage::setNodeType(int type, Id nodeId)
{
	executeStatement(
		"UPDATE node SET type = " + std::to_string(type) +
		" WHERE id == " + to_string(nodeId) + ";");
}

std::shared_ptr<SourceLocationFile> SqliteIndexStorage::getSourceLocationsForFile(
	const FilePath& filePath, const std::string& query) const
{
	std::shared_ptr<SourceLocationFile> ret = std::make_shared<SourceLocationFile>(
		filePath, "", true, false, false);

	const StorageFile file = getFileByPath(filePath.str());
	if (file.id == 0)	 // early out
	{
		return ret;
	}

	ret->setLanguage(file.languageIdentifier);
	ret->setIsComplete(file.complete);
	ret->setIsIndexed(file.indexed);

	std::vector<StorageSourceLocation> sourceLocations = doGetAll<StorageSourceLocation>(
		"WHERE file_node_id == " + to_string(file.id) + " " + query);

	std::vector<Id> sourceLocationIds;
	sourceLocationIds.reserve(sourceLocations.size());
	for (const StorageSourceLocation& storageLocation: sourceLocations)
	{
		sourceLocationIds.push_back(storageLocation.id);
	}

	std::map<Id, std::vector<Id>> sourceLocationIdToElementIds;
	for (const StorageOccurrence& occurrence: getOccurrencesForLocationIds(sourceLocationIds))
	{
		sourceLocationIdToElementIds[occurrence.sourceLocationId].push_back(occurrence.elementId);
	}

	for (const StorageSourceLocation& location: sourceLocations)
	{
		auto it = sourceLocationIdToElementIds.find(location.id);

		ret->addSourceLocation(
			location.type,
			location.id,
			it != sourceLocationIdToElementIds.end() ? it->second : std::vector<Id>(),
			location.startLine,
			location.startCol,
			location.endLine,
			location.endCol);
	}

	return ret;
}

std::shared_ptr<SourceLocationFile> SqliteIndexStorage::getSourceLocationsForLinesInFile(
	const FilePath& filePath, size_t startLine, size_t endLine) const
{
	return getSourceLocationsForFile(
		filePath,
		"AND start_line <= " + std::to_string(endLine) +
			" AND end_line >= " + std::to_string(startLine));
}

std::shared_ptr<SourceLocationFile> SqliteIndexStorage::getSourceLocationsOfTypeInFile(
	const FilePath& filePath, LocationType type) const
{
	return getSourceLocationsForFile(filePath, "AND type == " + to_string(type));
}

std::shared_ptr<SourceLocationCollection> SqliteIndexStorage::getSourceLocationsForElementIds(
	const std::vector<Id>& elementIds) const
{
	std::vector<Id> sourceLocationIds;
	std::map<Id, std::vector<Id>> sourceLocationIdToElementIds;
	for (const StorageOccurrence& occurrence: getOccurrencesForElementIds(elementIds))
	{
		sourceLocationIds.push_back(occurrence.sourceLocationId);
		sourceLocationIdToElementIds[occurrence.sourceLocationId].push_back(occurrence.elementId);
	}

	CppSQLite3Query q = executeQuery(
		"SELECT source_location.id, file.path, source_location.start_line, "
		"source_location.start_column, "
		"source_location.end_line, source_location.end_column, source_location.type "
		"FROM source_location INNER JOIN file ON (file.id = source_location.file_node_id) "
		"WHERE source_location.id IN (" +
		utility::join(utility::toStrings(sourceLocationIds), ',') + ");");

	std::shared_ptr<SourceLocationCollection> ret = std::make_shared<SourceLocationCollection>();

	while (!q.eof())
	{
		const Id id = q.getIntField(0, 0);
		const std::string filePath = q.getStringField(1, "");
		const auto startLineNumber = q.getIntField(2, -1);
		const auto startColNumber = q.getIntField(3, -1);
		const auto endLineNumber = q.getIntField(4, -1);
		const auto endColNumber = q.getIntField(5, -1);
		const auto type = q.getIntField(6, -1);

		if (id != 0 && filePath.size() && startLineNumber != -1 && startColNumber != -1 &&
			endLineNumber != -1 && endColNumber != -1 && type != -1)
		{
			ret->addSourceLocation(
				intToEnum<LocationType>(type),
				id,
				sourceLocationIdToElementIds[id],
				FilePath(filePath),
				startLineNumber,
				startColNumber,
				endLineNumber,
				endColNumber);
		}

		q.nextRow();
	}

	return ret;
}

std::vector<StorageOccurrence> SqliteIndexStorage::getOccurrencesForLocationId(Id locationId) const
{
	std::vector<Id> locationIds {locationId};
	return getOccurrencesForLocationIds(locationIds);
}

std::vector<StorageOccurrence> SqliteIndexStorage::getOccurrencesForLocationIds(
	const std::vector<Id>& locationIds) const
{
	return doGetAll<StorageOccurrence>(
		"WHERE source_location_id IN (" + utility::join(utility::toStrings(locationIds), ',') + ")");
}

std::vector<StorageOccurrence> SqliteIndexStorage::getOccurrencesForElementIds(
	const std::vector<Id>& elementIds) const
{
	return doGetAll<StorageOccurrence>(
		"WHERE element_id IN (" + utility::join(utility::toStrings(elementIds), ',') + ")");
}

StorageComponentAccess SqliteIndexStorage::getComponentAccessByNodeId(Id nodeId) const
{
	return doGetFirst<StorageComponentAccess>("WHERE node_id == " + to_string(nodeId));
}

std::vector<StorageComponentAccess> SqliteIndexStorage::getComponentAccessesByNodeIds(
	const std::vector<Id>& nodeIds) const
{
	return doGetAll<StorageComponentAccess>(
		"WHERE node_id IN (" + utility::join(utility::toStrings(nodeIds), ',') + ")");
}

std::vector<StorageElementComponent> SqliteIndexStorage::getElementComponentsByElementIds(
	const std::vector<Id>& elementIds) const
{
	return doGetAll<StorageElementComponent>(
		"WHERE element_id IN (" + utility::join(utility::toStrings(elementIds), ',') + ")");
}

std::vector<ErrorInfo> SqliteIndexStorage::getAllErrorInfos() const
{
	std::vector<ErrorInfo> errorInfos;

	CppSQLite3Query q = executeQuery(
		"SELECT error.id, error.message, error.fatal, error.indexed, error.translation_unit, "
		"file.path, source_location.start_line, source_location.start_column "
		"FROM occurrence "
		"INNER JOIN error ON (error.id = occurrence.element_id) "
		"INNER JOIN source_location ON (source_location.id = occurrence.source_location_id) "
		"INNER JOIN file ON (file.id = source_location.file_node_id);");

	std::map<Id, size_t> errorIdCount;

	while (!q.eof())
	{
		const Id id = q.getIntField(0, 0);
		const std::string message = q.getStringField(1, "");
		const bool fatal = q.getIntField(2, 0);
		const bool indexed = q.getIntField(3, 0);
		const std::string translationUnit = q.getStringField(4, "");
		const std::string filePath = q.getStringField(5, "");
		const auto lineNumber = q.getIntField(6, -1);
		const auto columnNumber = q.getIntField(7, -1);

		if (id != 0)
		{
			// There can be multiple errors with the same id, so a count is added to the id
			Id errorId = id * 10000;
			auto it = errorIdCount.find(id);
			if (it != errorIdCount.end())
			{
				errorId += it->second;
				it->second = it->second + 1;
			}
			else
			{
				errorIdCount.emplace(id, 1);
			}

			errorInfos.push_back(ErrorInfo(
				errorId,
				message,
				filePath,
				lineNumber,
				columnNumber,
				translationUnit,
				fatal,
				indexed));
		}

		q.nextRow();
	}

	return errorInfos;
}

int SqliteIndexStorage::getNodeCount() const
{
	return executeStatementScalar("SELECT COUNT(*) FROM node;");
}

int SqliteIndexStorage::getEdgeCount() const
{
	return executeStatementScalar("SELECT COUNT(*) FROM edge;");
}

int SqliteIndexStorage::getFileCount() const
{
	return executeStatementScalar("SELECT COUNT(*) FROM file WHERE indexed = 1;");
}

int SqliteIndexStorage::getCompletedFileCount() const
{
	return executeStatementScalar("SELECT COUNT(*) FROM file WHERE indexed = 1 AND complete = 1;");
}

int SqliteIndexStorage::getFileLineSum() const
{
	return executeStatementScalar("SELECT SUM(line_count) FROM file;");
}

int SqliteIndexStorage::getSourceLocationCount() const
{
	return executeStatementScalar("SELECT COUNT(*) FROM source_location;");
}

int SqliteIndexStorage::getErrorCount() const
{
	return executeStatementScalar("SELECT COUNT(*) FROM error INNER JOIN occurrence ON (error.id = occurrence.element_id);");
}

std::vector<std::pair<int, SqliteDatabaseIndex>> SqliteIndexStorage::getIndices() 
{
	std::vector<std::pair<int, SqliteDatabaseIndex>> indices;
	indices.push_back({STORAGE_MODE_CLEAR, SqliteDatabaseIndex("edge_source_node_id_index", "edge(source_node_id)")});
	indices.push_back({STORAGE_MODE_CLEAR, SqliteDatabaseIndex("edge_target_node_id_index", "edge(target_node_id)")});
	indices.push_back({STORAGE_MODE_READ | STORAGE_MODE_CLEAR, SqliteDatabaseIndex("node_serialized_name_index", "node(serialized_name)")});
	indices.push_back({STORAGE_MODE_READ | STORAGE_MODE_CLEAR, SqliteDatabaseIndex("source_location_file_node_id_index", "source_location(file_node_id)")});
	indices.push_back({STORAGE_MODE_WRITE, SqliteDatabaseIndex("error_all_data_index", "error(message, fatal)")});
	indices.push_back({STORAGE_MODE_WRITE, SqliteDatabaseIndex("file_path_index", "file(path)")});
	indices.push_back({STORAGE_MODE_READ | STORAGE_MODE_CLEAR, SqliteDatabaseIndex("occurrence_element_id_index", "occurrence(element_id)")});
	indices.push_back({STORAGE_MODE_READ | STORAGE_MODE_CLEAR, SqliteDatabaseIndex( "occurrence_source_location_id_index", "occurrence(source_location_id)")});
	indices.push_back({STORAGE_MODE_CLEAR, SqliteDatabaseIndex( "element_component_foreign_key_index", "element_component(element_id)")});
	indices.push_back({STORAGE_MODE_CLEAR, SqliteDatabaseIndex("edge_source_foreign_key_index", "edge(source_node_id)")});
	indices.push_back({STORAGE_MODE_CLEAR, SqliteDatabaseIndex("edge_target_foreign_key_index", "edge(target_node_id)")});
	indices.push_back({STORAGE_MODE_CLEAR, SqliteDatabaseIndex("source_location_foreign_key_index", "source_location(file_node_id)")});
	indices.push_back({STORAGE_MODE_CLEAR, SqliteDatabaseIndex("occurrence_element_foreign_key_index", "occurrence(element_id)")});
	indices.push_back({STORAGE_MODE_CLEAR, SqliteDatabaseIndex( "occurrence_source_location_foreign_key_index", "occurrence(source_location_id)")});

	return indices;
}

void SqliteIndexStorage::clearTables()
{
	try
	{
		m_database.execDML("DROP TABLE IF EXISTS main.error;");
		m_database.execDML("DROP TABLE IF EXISTS main.component_access;");
		m_database.execDML("DROP TABLE IF EXISTS main.occurrence;");
		m_database.execDML("DROP TABLE IF EXISTS main.source_location;");
		m_database.execDML("DROP TABLE IF EXISTS main.local_symbol;");
		m_database.execDML("DROP TABLE IF EXISTS main.filecontent;");
		m_database.execDML("DROP TABLE IF EXISTS main.file;");
		m_database.execDML("DROP TABLE IF EXISTS main.symbol;");
		m_database.execDML("DROP TABLE IF EXISTS main.node;");
		m_database.execDML("DROP TABLE IF EXISTS main.edge;");
		m_database.execDML("DROP TABLE IF EXISTS main.element_component;");
		m_database.execDML("DROP TABLE IF EXISTS main.element;");
		m_database.execDML("DROP TABLE IF EXISTS main.meta;");
	}
	catch (CppSQLite3Exception& e)
	{
		LOG_ERROR(std::to_string(e.errorCode()) + ": " + e.errorMessage());
	}
}

void SqliteIndexStorage::setupTables()
{
	try
	{
		m_database.execDML(
			"CREATE TABLE IF NOT EXISTS element("
			"id INTEGER, "
			"PRIMARY KEY(id));");

		m_database.execDML(
			"CREATE TABLE IF NOT EXISTS element_component("
			"	id INTEGER, "
			"	element_id INTEGER, "
			"	type INTEGER, "
			"	data TEXT, "
			"	PRIMARY KEY(id), "
			"	FOREIGN KEY(element_id) REFERENCES element(id) ON DELETE CASCADE"
			");");

		m_database.execDML(
			"CREATE TABLE IF NOT EXISTS edge("
			"id INTEGER NOT NULL, "
			"type INTEGER NOT NULL, "
			"source_node_id INTEGER NOT NULL, "
			"target_node_id INTEGER NOT NULL, "
			"PRIMARY KEY(id), "
			"FOREIGN KEY(id) REFERENCES element(id) ON DELETE CASCADE, "
			"FOREIGN KEY(source_node_id) REFERENCES node(id) ON DELETE CASCADE, "
			"FOREIGN KEY(target_node_id) REFERENCES node(id) ON DELETE CASCADE);");

		m_database.execDML(
			"CREATE TABLE IF NOT EXISTS node("
			"id INTEGER NOT NULL, "
			"type INTEGER NOT NULL, "
			"serialized_name TEXT, "
			"PRIMARY KEY(id), "
			"FOREIGN KEY(id) REFERENCES element(id) ON DELETE CASCADE);");

		m_database.execDML(
			"CREATE TABLE IF NOT EXISTS symbol("
			"id INTEGER NOT NULL, "
			"definition_kind INTEGER NOT NULL, "
			"PRIMARY KEY(id), "
			"FOREIGN KEY(id) REFERENCES node(id) ON DELETE CASCADE);");

		m_database.execDML(
			"CREATE TABLE IF NOT EXISTS file("
			"id INTEGER NOT NULL, "
			"path TEXT, "
			"language TEXT, "
			"modification_time TEXT, "
			"indexed INTEGER, "
			"complete INTEGER, "
			"line_count INTEGER, "
			"PRIMARY KEY(id), "
			"FOREIGN KEY(id) REFERENCES node(id) ON DELETE CASCADE);");

		m_database.execDML(
			"CREATE TABLE IF NOT EXISTS filecontent("
			"id INTEGER, "
			"content TEXT, "
			"PRIMARY KEY(id), "
			"FOREIGN KEY(id) REFERENCES file(id)"
			"ON DELETE CASCADE "
			"ON UPDATE CASCADE);");

		m_database.execDML(
			"CREATE TABLE IF NOT EXISTS local_symbol("
			"id INTEGER NOT NULL, "
			"name TEXT, "
			"PRIMARY KEY(id), "
			"FOREIGN KEY(id) REFERENCES element(id) ON DELETE CASCADE);");

		m_database.execDML(
			"CREATE TABLE IF NOT EXISTS source_location("
			"id INTEGER NOT NULL, "
			"file_node_id INTEGER, "
			"start_line INTEGER, "
			"start_column INTEGER, "
			"end_line INTEGER, "
			"end_column INTEGER, "
			"type INTEGER, "
			"PRIMARY KEY(id), "
			"FOREIGN KEY(file_node_id) REFERENCES node(id) ON DELETE CASCADE);");

		m_database.execDML(
			"CREATE TABLE IF NOT EXISTS occurrence("
			"element_id INTEGER NOT NULL, "
			"source_location_id INTEGER NOT NULL, "
			"PRIMARY KEY(element_id, source_location_id), "
			"FOREIGN KEY(element_id) REFERENCES element(id) ON DELETE CASCADE, "
			"FOREIGN KEY(source_location_id) REFERENCES source_location(id) ON DELETE CASCADE);");

		m_database.execDML(
			"CREATE TABLE IF NOT EXISTS component_access("
			"node_id INTEGER NOT NULL, "
			"type INTEGER NOT NULL, "
			"PRIMARY KEY(node_id), "
			"FOREIGN KEY(node_id) REFERENCES node(id) ON DELETE CASCADE);");

		m_database.execDML(
			"CREATE TABLE IF NOT EXISTS error("
			"id INTEGER NOT NULL, "
			"message TEXT, "
			"fatal INTEGER NOT NULL, "
			"indexed INTEGER NOT NULL, "
			"translation_unit TEXT, "
			"PRIMARY KEY(id), "
			"FOREIGN KEY(id) REFERENCES element(id) ON DELETE CASCADE);");
	}
	catch (CppSQLite3Exception& e)
	{
		LOG_ERROR(std::to_string(e.errorCode()) + ": " + e.errorMessage());

		throw(std::exception());
	}
}

void SqliteIndexStorage::setupPrecompiledStatements()
{
	try
	{
		m_insertNodeBatchStatement.compile(
			"INSERT INTO node(id, type, serialized_name) VALUES",
			3,
			[](CppSQLite3Statement& stmt, const StorageNode& node, size_t index) {
				stmt.bind(int(index) * 3 + 1, static_cast<Id::type>(node.id));
				stmt.bind(int(index) * 3 + 2, int(node.type));
				stmt.bind(int(index) * 3 + 3, node.serializedName);
			},
			m_database);
		m_insertEdgeBatchStatement.compile(
			"INSERT INTO edge(id, type, source_node_id, target_node_id) VALUES",
			4,
			[](CppSQLite3Statement& stmt, const StorageEdge& edge, size_t index) {
				stmt.bind(int(index) * 4 + 1, static_cast<Id::type>(edge.id));
				stmt.bind(int(index) * 4 + 2, int(edge.type));
				stmt.bind(int(index) * 4 + 3, static_cast<Id::type>(edge.sourceNodeId));
				stmt.bind(int(index) * 4 + 4, static_cast<Id::type>(edge.targetNodeId));
			},
			m_database);
		m_insertSymbolBatchStatement.compile(
			"INSERT OR IGNORE INTO symbol(id, definition_kind) VALUES",
			2,
			[](CppSQLite3Statement& stmt, const StorageSymbol& symbol, size_t index) {
				stmt.bind(int(index) * 2 + 1, static_cast<Id::type>(symbol.id));
				stmt.bind(int(index) * 2 + 2, int(symbol.definitionKind));
			},
			m_database);
		m_insertLocalSymbolBatchStatement.compile(
			"INSERT INTO local_symbol(id, name) VALUES",
			2,
			[](CppSQLite3Statement& stmt, const StorageLocalSymbol& symbol, size_t index) {
				stmt.bind(int(index) * 2 + 1, static_cast<Id::type>(symbol.id));
				stmt.bind(int(index) * 2 + 2, symbol.name);
			},
			m_database);
		m_insertSourceLocationBatchStatement.compile(
			"INSERT INTO source_location(file_node_id, start_line, start_column, end_line, "
			"end_column, type) VALUES",
			6,
			[](CppSQLite3Statement& stmt, const StorageSourceLocationData& location, size_t index) {
				stmt.bind(int(index) * 6 + 1, static_cast<Id::type>(location.fileNodeId));
				stmt.bind(int(index) * 6 + 2, int(location.startLine));
				stmt.bind(int(index) * 6 + 3, int(location.startCol));
				stmt.bind(int(index) * 6 + 4, int(location.endLine));
				stmt.bind(int(index) * 6 + 5, int(location.endCol));
				stmt.bind(int(index) * 6 + 6, int(location.type));
			},
			m_database);
		m_insertOccurrenceBatchStatement.compile(
			"INSERT OR IGNORE INTO occurrence(element_id, source_location_id) VALUES",
			2,
			[](CppSQLite3Statement& stmt, const StorageOccurrence& occurrence, size_t index) {
				stmt.bind(int(index) * 2 + 1, static_cast<Id::type>(occurrence.elementId));
				stmt.bind(int(index) * 2 + 2, static_cast<Id::type>(occurrence.sourceLocationId));
			},
			m_database);
		m_insertComponentAccessBatchStatement.compile(
			"INSERT OR IGNORE INTO component_access(node_id, type) VALUES",
			2,
			[](CppSQLite3Statement& stmt, const StorageComponentAccess& componentAccess, size_t index) {
				stmt.bind(int(index) * 2 + 1, static_cast<Id::type>(componentAccess.nodeId));
				stmt.bind(int(index) * 2 + 2, int(componentAccess.type));
			},
			m_database);

		m_insertElementStmt = m_database.compileStatement("INSERT INTO element(id) VALUES(NULL);");
		m_insertElementComponentStmt = m_database.compileStatement(
			"INSERT INTO element_component(id, element_id, type, data) VALUES(NULL, ?, ?, ?);");
		m_insertFileStmt = m_database.compileStatement(
			"INSERT INTO file(id, path, language, modification_time, indexed, complete, "
			"line_count) VALUES(?, ?, ?, ?, ?, ?, ?);");
		m_insertFileContentStmt = m_database.compileStatement(
			"INSERT INTO filecontent(id, content) VALUES(?, ?);");
		m_checkErrorExistsStmt = m_database.compileStatement(
			"SELECT id FROM error WHERE "
			"message = ? AND "
			"fatal == ? "
			"LIMIT 1;");
		m_insertErrorStmt = m_database.compileStatement(
			"INSERT INTO error(id, message, fatal, indexed, translation_unit) "
			"VALUES(?, ?, ?, ?, ?);");
	}
	catch (CppSQLite3Exception& e)
	{
		LOG_ERROR(std::to_string(e.errorCode()) + ": " + e.errorMessage());

		throw(std::exception());

		// todo: cancel project creation and destroy created files, display message
	}
}

template <>
void SqliteIndexStorage::forEach<StorageEdge>(
	const std::string& query, std::function<void(StorageEdge&&)> func) const
{
	CppSQLite3Query q = executeQuery(
		"SELECT id, type, source_node_id, target_node_id FROM edge " + query + ";");

	while (!q.eof())
	{
		const Id id = q.getIntField(0, 0);
		const auto type = q.getIntField(1, -1);
		const Id sourceId = q.getIntField(2, 0);
		const Id targetId = q.getIntField(3, 0);

		if (id != 0 && type != -1)
		{
			func(StorageEdge(id, intToEnum<Edge::EdgeType>(type), sourceId, targetId));
		}

		q.nextRow();
	}
}

template <>
void SqliteIndexStorage::forEach<StorageNode>(
	const std::string& query, std::function<void(StorageNode&&)> func) const
{
	CppSQLite3Query q = executeQuery("SELECT id, type, serialized_name FROM node " + query + ";");

	while (!q.eof())
	{
		const Id id = q.getIntField(0, 0);
		const auto type = q.getIntField(1, -1);
		const std::string serializedName = q.getStringField(2, "");

		if (id != 0 && type != -1)
		{
			func(StorageNode(id, intToEnum<NodeKind>(type), serializedName));
		}

		q.nextRow();
	}
}

template <>
void SqliteIndexStorage::forEach<StorageSymbol>(
	const std::string& query, std::function<void(StorageSymbol&&)> func) const
{
	CppSQLite3Query q = executeQuery("SELECT id, definition_kind FROM symbol " + query + ";");

	while (!q.eof())
	{
		const Id id = q.getIntField(0, 0);
		const auto definitionKind = q.getIntField(1, 0);

		if (id != 0)
		{
			func(StorageSymbol(id, intToEnum<DefinitionKind>(definitionKind)));
		}

		q.nextRow();
	}
}

template <>
void SqliteIndexStorage::forEach<StorageFile>(
	const std::string& query, std::function<void(StorageFile&&)> func) const
{
	CppSQLite3Query q = executeQuery(
		"SELECT id, path, language, modification_time, indexed, complete FROM file " + query + ";");

	while (!q.eof())
	{
		const Id id = q.getIntField(0, 0);
		const std::string filePath = q.getStringField(1, "");
		const std::string languageIdentifier = q.getStringField(2, "");
		const std::string modificationTime = q.getStringField(3, "");
		const bool indexed = q.getIntField(4, 0);
		const bool complete = q.getIntField(5, 0);

		if (id != 0)
		{
			func(StorageFile(
				id,
				filePath,
				languageIdentifier,
				modificationTime,
				indexed,
				complete));
		}
		q.nextRow();
	}
}

template <>
void SqliteIndexStorage::forEach<StorageLocalSymbol>(
	const std::string& query, std::function<void(StorageLocalSymbol&&)> func) const
{
	CppSQLite3Query q = executeQuery("SELECT id, name FROM local_symbol " + query + ";");

	while (!q.eof())
	{
		const Id id = q.getIntField(0, 0);
		const std::string name = q.getStringField(1, "");

		if (id != 0)
		{
			func(StorageLocalSymbol(id, name));
		}

		q.nextRow();
	}
}

template <>
void SqliteIndexStorage::forEach<StorageSourceLocation>(
	const std::string& query, std::function<void(StorageSourceLocation&&)> func) const
{
	CppSQLite3Query q = executeQuery(
		"SELECT id, file_node_id, start_line, start_column, end_line, end_column, type FROM "
		"source_location " +
		query + ";");

	while (!q.eof())
	{
		const Id id = q.getIntField(0, 0);
		const Id fileNodeId = q.getIntField(1, 0);
		const auto startLineNumber = q.getIntField(2, -1);
		const auto startColNumber = q.getIntField(3, -1);
		const auto endLineNumber = q.getIntField(4, -1);
		const auto endColNumber = q.getIntField(5, -1);
		const auto type = q.getIntField(6, -1);

		if (id != 0 && fileNodeId != 0 && startLineNumber != -1 && startColNumber != -1 &&
			endLineNumber != -1 && endColNumber != -1 && type != -1)
		{
			func(StorageSourceLocation(id, fileNodeId, startLineNumber, startColNumber, endLineNumber, 
				endColNumber, intToEnum<LocationType>(type)));
		}

		q.nextRow();
	}
}

template <>
void SqliteIndexStorage::forEach<StorageOccurrence>(
	const std::string& query, std::function<void(StorageOccurrence&&)> func) const
{
	CppSQLite3Query q = executeQuery(
		"SELECT element_id, source_location_id FROM occurrence " + query + ";");

	while (!q.eof())
	{
		const Id elementId = q.getIntField(0, 0);
		const Id sourceLocationId = q.getIntField(1, 0);

		if (elementId != 0 && sourceLocationId != 0)
		{
			func(StorageOccurrence(elementId, sourceLocationId));
		}

		q.nextRow();
	}
}

template <>
void SqliteIndexStorage::forEach<StorageComponentAccess>(
	const std::string& query, std::function<void(StorageComponentAccess&&)> func) const
{
	CppSQLite3Query q = executeQuery("SELECT node_id, type FROM component_access " + query + ";");

	while (!q.eof())
	{
		const Id nodeId = q.getIntField(0, 0);
		const auto type = q.getIntField(1, -1);

		if (nodeId != 0 && type != -1)
		{
			func(StorageComponentAccess(nodeId, intToEnum<AccessKind>(type)));
		}

		q.nextRow();
	}
}

template <>
void SqliteIndexStorage::forEach<StorageElementComponent>(
	const std::string& query, std::function<void(StorageElementComponent&&)> func) const
{
	CppSQLite3Query q = executeQuery(
		"SELECT element_id, type, data FROM element_component " + query + ";");

	while (!q.eof())
	{
		const Id elementId = q.getIntField(0, 0);
		const auto type = q.getIntField(1, -1);
		const std::string data = q.getStringField(2, "");

		if (elementId != 0 && type != -1)
		{
			func(StorageElementComponent(elementId, intToEnum<ElementComponentKind>(type), data));
		}

		q.nextRow();
	}
}

template <>
void SqliteIndexStorage::forEach<StorageError>(
	const std::string& query, std::function<void(StorageError&&)> func) const
{
	CppSQLite3Query q = executeQuery(
		"SELECT id, message, fatal, indexed, translation_unit FROM error " + query + ";");

	while (!q.eof())
	{
		const Id id = q.getIntField(0, 0);
		const std::string message = q.getStringField(1, "");
		const bool fatal = q.getIntField(2, 0);
		const bool indexed = q.getIntField(3, 0);
		const std::string translationUnit = q.getStringField(4, "");

		if (id != 0)
		{
			func(StorageError(
				id,
				message,
				translationUnit,
				fatal,
				indexed));
		}

		q.nextRow();
	}
}
