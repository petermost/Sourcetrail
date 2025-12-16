#ifndef SQLITE_BOOKMARK_STORAGE_H
#define SQLITE_BOOKMARK_STORAGE_H

#include "Bookmark.h"
#include "SqliteStorage.h"
#include "StorageBookmark.h"
#include "StorageBookmarkCategory.h"
#include "StorageBookmarkedEdge.h"
#include "StorageBookmarkedNode.h"

class SqliteBookmarkStorage: public SqliteStorage
{
public:
	SqliteBookmarkStorage(const FilePath& dbFilePath);

	size_t getStaticVersion() const override;

	StorageBookmarkCategory addBookmarkCategory(const StorageBookmarkCategoryData& data);
	StorageBookmark addBookmark(const StorageBookmarkData& data);
	StorageBookmarkedNode addBookmarkedNode(const StorageBookmarkedNodeData& data);
	StorageBookmarkedEdge addBookmarkedEdge(const StorageBookmarkedEdgeData data);

	void removeBookmarkCategory(Id id);
	void removeBookmark(const BookmarkId bookmarkId);

	std::vector<StorageBookmark> getAllBookmarks() const;
	std::vector<StorageBookmarkedNode> getAllBookmarkedNodes() const;
	std::vector<StorageBookmarkedEdge> getAllBookmarkedEdges() const;
	
	void updateBookmark(
		const BookmarkId bookmarkId, const std::string& name, const std::string& comment, const Id categoryId);

	std::vector<StorageBookmarkCategory> getAllBookmarkCategories() const;
	StorageBookmarkCategory getBookmarkCategoryByName(const std::string& name) const;

private:
	static const size_t s_storageVersion;

	virtual std::vector<std::pair<int, SqliteDatabaseIndex>> getIndices() const;
	void clearTables() override;
	void setupTables() override;
	void setupPrecompiledStatements() override;

	// void updateBookmarkMetaData(const BookmarkMetaData& metaData);

	template <typename ResultType>
	std::vector<ResultType> doGetAll(const std::string& query) const;

	template <typename ResultType>
	ResultType doGetFirst(const std::string& query) const
	{
		std::vector<ResultType> results = doGetAll<ResultType>(query + " LIMIT 1");
		if (results.size() > 0)
		{
			return results[0];
		}
		return ResultType();
	}
};

template <>
std::vector<StorageBookmarkCategory> SqliteBookmarkStorage::doGetAll<StorageBookmarkCategory>(
	const std::string& query) const;
template <>
std::vector<StorageBookmark> SqliteBookmarkStorage::doGetAll<StorageBookmark>(
	const std::string& query) const;
template <>
std::vector<StorageBookmarkedNode> SqliteBookmarkStorage::doGetAll<StorageBookmarkedNode>(
	const std::string& query) const;
template <>
std::vector<StorageBookmarkedEdge> SqliteBookmarkStorage::doGetAll<StorageBookmarkedEdge>(
	const std::string& query) const;

#endif	  // SQLITE_BOOKMARK_STORAGE_H
