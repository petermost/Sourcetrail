#ifndef STORAGE_BOOKMARK_H
#define STORAGE_BOOKMARK_H

#include <string>

#include "Bookmark.h"

struct StorageBookmarkData
{
	StorageBookmarkData():  categoryId(0) {}

	StorageBookmarkData(
		const std::string& name,
		const std::string& comment,
		const std::string& timestamp,
		const Id categoryId)
		: name(name), comment(comment), timestamp(timestamp), categoryId(categoryId)
	{
	}

	std::string name;
	std::string comment;
	std::string timestamp;
	Id categoryId;
};

struct StorageBookmark: public StorageBookmarkData
{
	StorageBookmark():  bookmarkId(BookmarkId::NONE) {}

	StorageBookmark(BookmarkId bookmarkId, const StorageBookmarkData& data): StorageBookmarkData(data), bookmarkId(bookmarkId) {}

	StorageBookmark(
		BookmarkId bookmarkId,
		const std::string& name,
		const std::string& comment,
		const std::string& timestamp,
		const Id categoryId)
		: StorageBookmarkData(name, comment, timestamp, categoryId), bookmarkId(bookmarkId)
	{
	}

	BookmarkId bookmarkId;
};

#endif	  // STORAGE_BOOKMARK_H
