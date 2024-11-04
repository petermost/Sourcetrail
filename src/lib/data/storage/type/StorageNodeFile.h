#ifndef STORAGE_NODE_FILE_H
#define STORAGE_NODE_FILE_H

#include <string>

#include "types.h"

struct StorageNodeFile
{
	StorageNodeFile(): id(0), fileName("") {}
	StorageNodeFile(Id id, std::string fileName, bool display_content)
		: id(id), fileName(fileName), display_content(display_content)
	{
	}

	bool operator<(const StorageNodeFile& other) const
	{
		return id < other.id;
	}

	Id id;
	std::string fileName;
	bool display_content;
};

#endif	  // STORAGE_NODE_FILE_H