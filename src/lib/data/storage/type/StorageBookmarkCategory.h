#ifndef STORAGE_BOOKMARK_CATEGORY_H
#define STORAGE_BOOKMARK_CATEGORY_H

#include <string>

#include "types.h"

struct StorageBookmarkCategoryData
{
	StorageBookmarkCategoryData() = default;

	StorageBookmarkCategoryData(const std::string& name): name(name) {}

	std::string name;
};

struct StorageBookmarkCategory: public StorageBookmarkCategoryData
{
	StorageBookmarkCategory():  id(0) {}

	StorageBookmarkCategory(Id id, const StorageBookmarkCategoryData& data)
		: StorageBookmarkCategoryData(data), id(id)
	{
	}

	StorageBookmarkCategory(Id id, const std::string& name)
		: StorageBookmarkCategoryData(name), id(id)
	{
	}

	Id id;
};

#endif	  // STORAGE_BOOKMARK_CATEGORY_H
