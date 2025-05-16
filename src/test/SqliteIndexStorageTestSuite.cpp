#include "Catch2.hpp"

#include "FileSystem.h"
#include "SqliteIndexStorage.h"

TEST_CASE("storage adds node successfully")
{
	FilePath databasePath("data/SQLiteTestSuite/test.sqlite");
	int nodeCount = -1;
	{
		SqliteIndexStorage storage(databasePath);
		storage.setup();
		storage.beginTransaction();
		storage.addNode(StorageNodeData(NODE_UNDEFINED, "a"));
		storage.commitTransaction();
		nodeCount = storage.getNodeCount();
	}
	FileSystem::remove(databasePath);

	REQUIRE(1 == nodeCount);
}

TEST_CASE("storage removes node successfully")
{
	FilePath databasePath("data/SQLiteTestSuite/test.sqlite");
	int nodeCount = -1;
	{
		SqliteIndexStorage storage(databasePath);
		storage.setup();
		storage.beginTransaction();
		Id nodeId = storage.addNode(StorageNodeData(NODE_UNDEFINED, "a"));
		storage.removeElement(nodeId);
		storage.commitTransaction();
		nodeCount = storage.getNodeCount();
	}
	FileSystem::remove(databasePath);

	REQUIRE(0 == nodeCount);
}

TEST_CASE("storage adds edge successfully")
{
	FilePath databasePath("data/SQLiteTestSuite/test.sqlite");
	int edgeCount = -1;
	{
		SqliteIndexStorage storage(databasePath);
		storage.setup();
		storage.beginTransaction();
		Id sourceNodeId = storage.addNode(StorageNodeData(NODE_UNDEFINED, "a"));
		Id targetNodeId = storage.addNode(StorageNodeData(NODE_UNDEFINED, "b"));
		storage.addEdge(StorageEdgeData(Edge::EDGE_UNDEFINED, sourceNodeId, targetNodeId));
		storage.commitTransaction();
		edgeCount = storage.getEdgeCount();
	}
	FileSystem::remove(databasePath);

	REQUIRE(1 == edgeCount);
}

TEST_CASE("storage removes edge successfully")
{
	FilePath databasePath("data/SQLiteTestSuite/test.sqlite");
	int edgeCount = -1;
	{
		SqliteIndexStorage storage(databasePath);
		storage.setup();
		storage.beginTransaction();
		Id sourceNodeId = storage.addNode(StorageNodeData(NODE_UNDEFINED, "a"));
		Id targetNodeId = storage.addNode(StorageNodeData(NODE_UNDEFINED, "b"));
		Id edgeId = storage.addEdge(StorageEdgeData(Edge::EDGE_UNDEFINED, sourceNodeId, targetNodeId));
		storage.removeElement(edgeId);
		storage.commitTransaction();
		edgeCount = storage.getEdgeCount();
	}
	FileSystem::remove(databasePath);

	REQUIRE(0 == edgeCount);
}
