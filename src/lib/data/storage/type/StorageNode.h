#ifndef STORAGE_NODE_H
#define STORAGE_NODE_H

#include <string>

#include "Id.h"
#include "NodeKind.h"

struct StorageNodeData
{
	StorageNodeData(): type(NODE_UNDEFINED) {}

	StorageNodeData(NodeKind type, std::string serializedName)
		: type(type), serializedName(std::move(serializedName))
	{
	}

	bool operator<(const StorageNodeData& other) const
	{
		return serializedName < other.serializedName;
	}

	NodeKind type;
	std::string serializedName;
};

struct StorageNode: public StorageNodeData
{
	StorageNode():  id(0) {}

	StorageNode(Id id, NodeKind type, std::string serializedName)
		: StorageNodeData(type, std::move(serializedName)), id(id)
	{
	}

	StorageNode(Id id, const StorageNodeData& data): StorageNodeData(data), id(id) {}

	Id id;
};

#endif	  // STORAGE_NODE_H
