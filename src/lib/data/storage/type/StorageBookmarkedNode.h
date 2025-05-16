#ifndef STORAGE_BOOKMARKED_NODE_H
#define STORAGE_BOOKMARKED_NODE_H

#include <string>

#include "Bookmark.h"

struct StorageBookmarkedNodeData
{
	StorageBookmarkedNodeData(): bookmarkId(BookmarkId::NONE) {}

	StorageBookmarkedNodeData(BookmarkId bookmarkId, const std::string& serializedNodeName)
		: bookmarkId(bookmarkId), serializedNodeName(serializedNodeName)
	{
	}

	BookmarkId bookmarkId;
	std::string serializedNodeName;
};

struct StorageBookmarkedNode: public StorageBookmarkedNodeData
{
	StorageBookmarkedNode():  id(0) {}

	StorageBookmarkedNode(Id id, const StorageBookmarkedNodeData& data)
		: StorageBookmarkedNodeData(data), id(id)
	{
	}

	StorageBookmarkedNode(Id id, BookmarkId bookmarkId, const std::string& serializedNodeName)
		: StorageBookmarkedNodeData(bookmarkId, serializedNodeName), id(id)
	{
	}

	Id id;
};

#endif	  // STORAGE_BOOKMARKED_NODE_H
