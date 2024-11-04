#ifndef STORAGE_NODE_TYPE_H
#define STORAGE_NODE_TYPE_H

#include <string>

#include "types.h"

struct StorageNodeType
{
	StorageNodeType(): id(0), graphDisplay(""), hoverDisplay("") {}
	StorageNodeType(Id id, std::string graphDisplay, std::string hoverDisplay): id(id), graphDisplay(graphDisplay), hoverDisplay(hoverDisplay) {}

	bool operator<(const StorageNodeType& other) const
	{
		return id < other.id;
	}

	Id id;
	std::string graphDisplay;
	std::string hoverDisplay;
};

#endif	  // STORAGE_NODE_TYPE_H