#ifndef BOOKMARK_CONTROLLER_H
#define BOOKMARK_CONTROLLER_H

#include "ActivationListener.h"
#include "Bookmark.h"
#include "Controller.h"
#include "EdgeBookmark.h"
#include "MessageBookmarkActivate.h"
#include "MessageBookmarkBrowse.h"
#include "MessageBookmarkCreate.h"
#include "MessageBookmarkDelete.h"
#include "MessageBookmarkEdit.h"
#include "MessageBookmarkReferences.h"
#include "MessageBookmarkReferencing.h"
#include "MessageIndexingFinished.h"
#include "MessageListener.h"
#include "NodeBookmark.h"

class StorageAccess;

class BookmarkController
	: public Controller
	, public ActivationListener
	, public MessageListener<MessageBookmarkActivate>
	, public MessageListener<MessageBookmarkBrowse>
	, public MessageListener<MessageBookmarkCreate>
	, public MessageListener<MessageBookmarkDelete>
	, public MessageListener<MessageBookmarkEdit>
	, public MessageListener<MessageBookmarkReferences>
	, public MessageListener<MessageBookmarkReferencing>
	, public MessageListener<MessageIndexingFinished>
{
public:
	BookmarkController(StorageAccess* storageAccess);
	~BookmarkController() override;

	void clear() override;

	void displayBookmarks();
	void displayBookmarksFor(Bookmark::BookmarkFilter filter, Bookmark::BookmarkOrder order);

	void createBookmark(
		const std::wstring& name, const std::wstring& comment, const std::wstring& category, Id nodeId);
	void editBookmark(
		Id bookmarkId,
		const std::wstring& name,
		const std::wstring& comment,
		const std::wstring& category);

	void deleteBookmark(Id bookmarkId);
	void deleteBookmarkCategory(Id categoryId);
	void deleteBookmarkForActiveTokens();

	void activateBookmark(const std::shared_ptr<Bookmark> bookmark);

	void showBookmarkCreator(Id nodeId = 0);
	void showBookmarkEditor(const std::shared_ptr<Bookmark> bookmark);

	void bookmarkReferences(Id nodeId);
	void bookmarkReferencing(Id nodeId);

private:
	class BookmarkCache
	{
	public:
		BookmarkCache(StorageAccess* storageAccess);

		void clear();

		std::vector<NodeBookmark> getAllNodeBookmarks();
		std::vector<EdgeBookmark> getAllEdgeBookmarks();

	private:
		StorageAccess* m_storageAccess;
		std::vector<NodeBookmark> m_nodeBookmarks;
		std::vector<EdgeBookmark> m_edgeBookmarks;
		bool m_nodeBookmarksValid;
		bool m_edgeBookmarksValid;
	};

	void handleActivation(const MessageActivateBase* message) override;

	void handleMessage(MessageActivateTokens* message) override;
	void handleMessage(MessageBookmarkActivate* message) override;
	void handleMessage(MessageBookmarkBrowse* message) override;
	void handleMessage(MessageBookmarkCreate* message) override;
	void handleMessage(MessageBookmarkDelete* message) override;
	void handleMessage(MessageBookmarkEdit* message) override;
	void handleMessage(MessageBookmarkReferences* message) override;
	void handleMessage(MessageBookmarkReferencing* message) override;
	void handleMessage(MessageIndexingFinished* message) override;

	std::vector<std::wstring> getActiveTokenDisplayNames() const;
	std::vector<std::wstring> getDisplayNamesForNodeId(Id nodeId) const;

	std::vector<BookmarkCategory> getAllBookmarkCategories() const;

	std::shared_ptr<Bookmark> getBookmarkForActiveToken(Id tabId) const;
	std::shared_ptr<Bookmark> getBookmarkForNodeId(Id nodeId) const;

	bool canCreateBookmark() const;

	std::vector<std::shared_ptr<Bookmark>> getAllBookmarks() const;
	std::vector<std::shared_ptr<NodeBookmark>> getAllNodeBookmarks() const;
	std::vector<std::shared_ptr<EdgeBookmark>> getAllEdgeBookmarks() const;
	std::vector<std::shared_ptr<Bookmark>> getBookmarks(
		Bookmark::BookmarkFilter filter, Bookmark::BookmarkOrder order) const;

	std::vector<std::wstring> getActiveNodeDisplayNames() const;
	std::vector<std::wstring> getActiveEdgeDisplayNames() const;
	std::wstring getNodeDisplayName(const Id id) const;

	static std::vector<std::shared_ptr<Bookmark>> getFilteredBookmarks(
		const std::vector<std::shared_ptr<Bookmark>>& bookmarks,
		Bookmark::BookmarkFilter filter);
	static std::vector<std::shared_ptr<Bookmark>> getOrderedBookmarks(
		const std::vector<std::shared_ptr<Bookmark>>& bookmarks, Bookmark::BookmarkOrder order);
	static std::vector<std::shared_ptr<Bookmark>> getDateOrderedBookmarks(
		const std::vector<std::shared_ptr<Bookmark>>& bookmarks, const bool ascending);
	static std::vector<std::shared_ptr<Bookmark>> getNameOrderedBookmarks(
		const std::vector<std::shared_ptr<Bookmark>>& bookmarks, const bool ascending);

	void cleanBookmarkCategories();

	static bool bookmarkDateCompare(
		const std::shared_ptr<Bookmark> a, const std::shared_ptr<Bookmark> b);
	static bool bookmarkNameCompare(
		const std::shared_ptr<Bookmark> a, const std::shared_ptr<Bookmark> b);

	void update();

	static const std::wstring s_edgeSeparatorToken;
	static const std::wstring s_defaultCategoryName;

	StorageAccess* m_storageAccess;
	mutable BookmarkCache m_bookmarkCache;

	mutable std::map<Id, std::vector<Id>> m_activeNodeIds;
	mutable std::map<Id, std::vector<Id>> m_activeEdgeIds;

	Bookmark::BookmarkFilter m_filter = Bookmark::FILTER_ALL;
	Bookmark::BookmarkOrder m_order = Bookmark::ORDER_DATE_DESCENDING;
};

#endif	  // BOOKMARK_CONTROLLER_H
