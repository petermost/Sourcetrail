#include "BookmarkController.h"

#include "Application.h"
#include "Bookmark.h"
#include "BookmarkView.h"
#include "EdgeBookmark.h"
#include "NodeBookmark.h"
#include "StorageAccess.h"
#include "StorageEdge.h"

#include "MessageActivateEdge.h"
#include "MessageActivateNodes.h"
#include "MessageBookmarkButtonState.h"
#include "TabIds.h"
#include "logging.h"
#include "utility.h"
#include "utilityString.h"

using namespace utility;

const std::string BookmarkController::s_edgeSeparatorToken = " => ";
const std::string BookmarkController::s_defaultCategoryName = "default";

BookmarkController::BookmarkController(StorageAccess* storageAccess)
	: m_storageAccess(storageAccess)
	, m_bookmarkCache(storageAccess)

{
}

BookmarkController::~BookmarkController() = default;

void BookmarkController::clear()
{
	m_activeNodeIds.clear();
	m_activeEdgeIds.clear();
}

void BookmarkController::displayBookmarks()
{
	getView<BookmarkView>()->displayBookmarkBrowser(getBookmarks(m_filter, m_order));
}

void BookmarkController::undisplayBookmarks()
{
	getView<BookmarkView>()->undisplayBookmarkBrowser();
}


void BookmarkController::displayBookmarksFor(
	Bookmark::BookmarkFilter filter, Bookmark::BookmarkOrder order)
{
	if (filter != Bookmark::FILTER_UNKNOWN)
	{
		m_filter = filter;
	}

	if (order != Bookmark::ORDER_NONE)
	{
		m_order = order;
	}

	displayBookmarks();
}

void BookmarkController::createBookmark(
	const std::string& name, const std::string& comment, const std::string& category, Id nodeId)
{
	LOG_INFO("Attempting to create new bookmark");

	TabId tabId = TabIds::currentTab();

	BookmarkCategory bookmarkCategory(0, category.empty() ? s_defaultCategoryName : category);

	if (!m_activeEdgeIds[tabId].empty())
	{
		LOG_INFO("Creating Edge Bookmark");

		EdgeBookmark bookmark(BookmarkId::NONE, name, comment, TimeStamp::now(), bookmarkCategory);
		bookmark.setEdgeIds(m_activeEdgeIds[tabId]);

		if (!m_activeNodeIds[TabIds::currentTab()].empty())
		{
			bookmark.setActiveNodeId(m_activeNodeIds[tabId].front());
		}
		else
		{
			LOG_ERROR("Cannot create bookmark for edge if no active node exists");
		}

		m_storageAccess->addEdgeBookmark(bookmark);
	}
	else
	{
		LOG_INFO("Creating Node Bookmark");

		NodeBookmark bookmark(BookmarkId::NONE, name, comment, TimeStamp::now(), bookmarkCategory);
		if (nodeId)
		{
			bookmark.addNodeId(nodeId);
		}
		else
		{
			bookmark.setNodeIds(m_activeNodeIds[TabIds::currentTab()]);
		}

		m_storageAccess->addNodeBookmark(bookmark);
	}

	m_bookmarkCache.clear();

	if (!nodeId ||
		(m_activeNodeIds[TabIds::currentTab()].size() == 1 &&
		 m_activeNodeIds[TabIds::currentTab()][0] == nodeId))
	{
		MessageBookmarkButtonState(TabIds::currentTab(), MessageBookmarkButtonState::ALREADY_CREATED)
			.dispatch();
	}

	update();
}

void BookmarkController::editBookmark(
	BookmarkId bookmarkId, const std::string& name, const std::string& comment, const std::string& category)
{
	LOG_INFO_STREAM(<< "Attempting to update Bookmark " << bookmarkId);

	m_storageAccess->updateBookmark(
		bookmarkId, name, comment, category.size() ? category : s_defaultCategoryName);

	cleanBookmarkCategories();

	update();
}

void BookmarkController::deleteBookmark(BookmarkId bookmarkId)
{
	LOG_INFO_STREAM(<< "Attempting to delete Bookmark " << bookmarkId);

	m_storageAccess->removeBookmark(bookmarkId);

	cleanBookmarkCategories();

	if (!getBookmarkForActiveToken(TabIds::currentTab()))
	{
		MessageBookmarkButtonState(TabIds::currentTab(), MessageBookmarkButtonState::CAN_CREATE)
			.dispatch();
	}

	update();
}

void BookmarkController::deleteBookmarkCategory(Id categoryId)
{
	m_storageAccess->removeBookmarkCategory(categoryId);

	m_bookmarkCache.clear();

	if (!getBookmarkForActiveToken(TabIds::currentTab()))
	{
		MessageBookmarkButtonState(TabIds::currentTab(), MessageBookmarkButtonState::CAN_CREATE)
			.dispatch();
	}

	update();
}

void BookmarkController::deleteBookmarkForActiveTokens()
{
	if (std::shared_ptr<Bookmark> bookmark = getBookmarkForActiveToken(TabIds::currentTab()))
	{
		LOG_INFO("Deleting bookmark " + bookmark->getName());

		m_storageAccess->removeBookmark(bookmark->getId());

		cleanBookmarkCategories();

		MessageBookmarkButtonState(TabIds::currentTab(), MessageBookmarkButtonState::CAN_CREATE)
			.dispatch();
		update();
	}
	else
	{
		LOG_WARNING("No Bookmark to delete for active tokens.");
	}
}

void BookmarkController::activateBookmark(const std::shared_ptr<Bookmark> bookmark)
{
	LOG_INFO("Attempting to activate Bookmark");

	if (std::shared_ptr<EdgeBookmark> edgeBookmark = std::dynamic_pointer_cast<EdgeBookmark>(bookmark))
	{
		if (!edgeBookmark->getEdgeIds().empty())
		{
			const Id firstEdgeId = edgeBookmark->getEdgeIds().front();
			const StorageEdge storageEdge = m_storageAccess->getEdgeById(firstEdgeId);

			const NameHierarchy sourceName = m_storageAccess->getNameHierarchyForNodeId(
				storageEdge.sourceNodeId);
			const NameHierarchy targetName = m_storageAccess->getNameHierarchyForNodeId(
				storageEdge.targetNodeId);

			if (edgeBookmark->getEdgeIds().size() == 1)
			{
				Id activeNodeId = edgeBookmark->getActiveNodeId();
				if (activeNodeId)
				{
					MessageActivateNodes(activeNodeId).dispatch();
				}
				MessageActivateEdge(firstEdgeId, storageEdge.type, sourceName, targetName).dispatch();
			}
			else
			{
				MessageActivateEdge activateEdge(
					0, Edge::EdgeType::EDGE_BUNDLED_EDGES, sourceName, targetName);
				for (const Id bundledEdgeId: edgeBookmark->getEdgeIds())
				{
					activateEdge.bundledEdgesIds.push_back(bundledEdgeId);
				}
				activateEdge.dispatch();
			}
		}
		else
		{
			LOG_ERROR("Failed to activate bookmark, did not find edges to activate");
		}
	}
	else if (std::shared_ptr<NodeBookmark> nodeBookmark = std::dynamic_pointer_cast<NodeBookmark>(bookmark))
	{
		MessageActivateNodes activateNodes;

		for (Id nodeId: nodeBookmark->getNodeIds())
		{
			activateNodes.addNode(nodeId);
		}

		activateNodes.dispatch();
	}
}

void BookmarkController::showBookmarkCreator(Id nodeId)
{
	TabId tabId = TabIds::currentTab();
	if (!m_activeNodeIds[tabId].size() && !m_activeEdgeIds[tabId].size() && !nodeId)
	{
		return;
	}

	BookmarkView* view = getView<BookmarkView>();

	if (nodeId)
	{
		std::shared_ptr<Bookmark> bookmark = getBookmarkForNodeId(nodeId);
		if (bookmark != nullptr)
		{
			view->displayBookmarkCreator(bookmark, getAllBookmarkCategories());
		}
		else
		{
			view->displayBookmarkCreator(getDisplayNamesForNodeId(nodeId), getAllBookmarkCategories(), nodeId);
		}
	}
	else
	{
		std::shared_ptr<Bookmark> bookmark = getBookmarkForActiveToken(tabId);
		if (bookmark != nullptr)
		{
			view->displayBookmarkCreator(bookmark, getAllBookmarkCategories());
		}
		else
		{
			view->displayBookmarkCreator(getActiveTokenDisplayNames(), getAllBookmarkCategories(), 0);
		}
	}
}

void BookmarkController::showBookmarkEditor(const std::shared_ptr<Bookmark> bookmark)
{
	getView<BookmarkView>()->displayBookmarkCreator(bookmark, getAllBookmarkCategories());
}

BookmarkController::BookmarkCache::BookmarkCache(StorageAccess* storageAccess)
	: m_storageAccess(storageAccess)
{
}

void BookmarkController::BookmarkCache::clear()
{
	m_nodeBookmarksValid = false;
	m_edgeBookmarksValid = false;
}

std::vector<NodeBookmark> BookmarkController::BookmarkCache::getAllNodeBookmarks()
{
	if (!m_nodeBookmarksValid)
	{
		m_nodeBookmarks = m_storageAccess->getAllNodeBookmarks();
		m_nodeBookmarksValid = true;
	}
	return m_nodeBookmarks;
}

std::vector<EdgeBookmark> BookmarkController::BookmarkCache::getAllEdgeBookmarks()
{
	if (!m_edgeBookmarksValid)
	{
		m_edgeBookmarks = m_storageAccess->getAllEdgeBookmarks();
		m_edgeBookmarksValid = true;
	}
	return m_edgeBookmarks;
}

void BookmarkController::handleActivation(const MessageActivateBase*  /*message*/)
{
	clear();
}

void BookmarkController::handleMessage(MessageActivateTokens* message)
{
	TabId tabId = message->getSchedulerId();
	m_activeEdgeIds[tabId].clear();

	if (message->isEdge || message->isBundledEdges)
	{
		m_activeEdgeIds[tabId] = message->tokenIds;

		if (getBookmarkForActiveToken(tabId))
		{
			MessageBookmarkButtonState(tabId, MessageBookmarkButtonState::ALREADY_CREATED).dispatch();
		}
		else
		{
			MessageBookmarkButtonState(tabId, MessageBookmarkButtonState::CAN_CREATE).dispatch();
		}
	}
	else if (!message->isEdge)
	{
		m_activeNodeIds[tabId] = message->tokenIds;

		if (getBookmarkForActiveToken(tabId))
		{
			MessageBookmarkButtonState(tabId, MessageBookmarkButtonState::ALREADY_CREATED).dispatch();
		}
		else
		{
			MessageBookmarkButtonState(tabId, MessageBookmarkButtonState::CAN_CREATE).dispatch();
		}
	}
}

void BookmarkController::handleMessage(MessageBookmarkActivate* message)
{
	activateBookmark(message->bookmark);
}

void BookmarkController::handleMessage(MessageBookmarkBrowse* message)
{
	displayBookmarksFor(message->filter, message->order);
}

void BookmarkController::handleMessage(MessageBookmarkCreate* message)
{
	showBookmarkCreator(message->nodeId);
}

void BookmarkController::handleMessage(MessageBookmarkDelete*  /*message*/)
{
	deleteBookmarkForActiveTokens();
}

void BookmarkController::handleMessage(MessageBookmarkEdit*  /*message*/)
{
	showBookmarkCreator(0);
}

void BookmarkController::handleMessage(MessageIndexingFinished*  /*message*/)
{
	m_bookmarkCache.clear();

	update();
}

std::vector<std::string> BookmarkController::getActiveTokenDisplayNames() const
{
	if (m_activeEdgeIds[TabIds::currentTab()].size() > 0)
	{
		return getActiveEdgeDisplayNames();
	}
	else
	{
		return getActiveNodeDisplayNames();
	}
}

std::vector<std::string> BookmarkController::getDisplayNamesForNodeId(Id nodeId) const
{
	return std::vector<std::string>({getNodeDisplayName(nodeId)});
}

std::vector<BookmarkCategory> BookmarkController::getAllBookmarkCategories() const
{
	return m_storageAccess->getAllBookmarkCategories();
}

std::shared_ptr<Bookmark> BookmarkController::getBookmarkForActiveToken(TabId tabId) const
{
	if (!m_activeEdgeIds[tabId].empty())
	{
		for (const std::shared_ptr<EdgeBookmark>& edgeBookmark: getAllEdgeBookmarks())
		{
			if (!m_activeNodeIds[tabId].empty() &&
				edgeBookmark->getActiveNodeId() == m_activeNodeIds[tabId].front() &&
				utility::isPermutation(edgeBookmark->getEdgeIds(), m_activeEdgeIds[tabId]))
			{
				return std::make_shared<EdgeBookmark>(*edgeBookmark);
			}
		}
	}
	else
	{
		for (const std::shared_ptr<NodeBookmark>& nodeBookmark: getAllNodeBookmarks())
		{
			if (utility::isPermutation(nodeBookmark->getNodeIds(), m_activeNodeIds[tabId]))
			{
				return std::make_shared<NodeBookmark>(*nodeBookmark);
			}
		}
	}

	return std::shared_ptr<Bookmark>();
}

std::shared_ptr<Bookmark> BookmarkController::getBookmarkForNodeId(Id nodeId) const
{
	for (const std::shared_ptr<NodeBookmark>& nodeBookmark: getAllNodeBookmarks())
	{
		if (nodeBookmark->getNodeIds().size() == 1 && nodeBookmark->getNodeIds()[0] == nodeId)
		{
			return std::make_shared<NodeBookmark>(*nodeBookmark);
		}
	}

	return std::shared_ptr<Bookmark>();
}

std::vector<std::shared_ptr<Bookmark>> BookmarkController::getAllBookmarks() const
{
	LOG_INFO("Retrieving all bookmarks");

	std::vector<std::shared_ptr<Bookmark>> bookmarks;

	for (const std::shared_ptr<NodeBookmark>& nodeBookmark: getAllNodeBookmarks())
	{
		bookmarks.push_back(nodeBookmark);
	}
	for (const std::shared_ptr<EdgeBookmark>& edgeBookmark: getAllEdgeBookmarks())
	{
		bookmarks.push_back(edgeBookmark);
	}

	return bookmarks;
}

std::vector<std::shared_ptr<NodeBookmark>> BookmarkController::getAllNodeBookmarks() const
{
	std::vector<std::shared_ptr<NodeBookmark>> bookmarks;
	for (const NodeBookmark& nodeBookmark: m_bookmarkCache.getAllNodeBookmarks())
	{
		bookmarks.push_back(std::make_shared<NodeBookmark>(nodeBookmark));
	}
	return bookmarks;
}

std::vector<std::shared_ptr<EdgeBookmark>> BookmarkController::getAllEdgeBookmarks() const
{
	std::vector<std::shared_ptr<EdgeBookmark>> bookmarks;
	for (const EdgeBookmark& edgeBookmark: m_bookmarkCache.getAllEdgeBookmarks())
	{
		bookmarks.push_back(std::make_shared<EdgeBookmark>(edgeBookmark));
	}
	return bookmarks;
}

std::vector<std::shared_ptr<Bookmark>> BookmarkController::getBookmarks(
	Bookmark::BookmarkFilter filter, Bookmark::BookmarkOrder order) const
{
	LOG_INFO_STREAM(
		<< "Retrieving bookmarks with filter \"" << filter << "\" and order \"" << order << "\"");

	std::vector<std::shared_ptr<Bookmark>> bookmarks = getAllBookmarks();
	bookmarks = getFilteredBookmarks(bookmarks, filter);
	bookmarks = getOrderedBookmarks(bookmarks, order);
	return bookmarks;
}

std::vector<std::string> BookmarkController::getActiveNodeDisplayNames() const
{
	std::vector<std::string> names;
	for (Id nodeId: m_activeNodeIds[TabIds::currentTab()])
	{
		names.push_back(getNodeDisplayName(nodeId));
	}
	return names;
}

std::vector<std::string> BookmarkController::getActiveEdgeDisplayNames() const
{
	std::vector<std::string> activeEdgeDisplayNames;
	for (Id activeEdgeId: m_activeEdgeIds[TabIds::currentTab()])
	{
		const StorageEdge activeEdge = m_storageAccess->getEdgeById(activeEdgeId);
		const std::string sourceDisplayName = getNodeDisplayName(activeEdge.sourceNodeId);
		const std::string targetDisplayName = getNodeDisplayName(activeEdge.targetNodeId);
		activeEdgeDisplayNames.push_back(sourceDisplayName + s_edgeSeparatorToken + targetDisplayName);
	}
	return activeEdgeDisplayNames;
}

std::string BookmarkController::getNodeDisplayName(const Id nodeId) const
{
	NodeType type = m_storageAccess->getNodeTypeForNodeWithId(nodeId);
	NameHierarchy nameHierarchy = m_storageAccess->getNameHierarchyForNodeId(nodeId);

	if (type.isFile())
	{
		return FilePath(nameHierarchy.getQualifiedName()).fileName();
	}

	return nameHierarchy.getQualifiedName();
}

std::vector<std::shared_ptr<Bookmark>> BookmarkController::getFilteredBookmarks(
	const std::vector<std::shared_ptr<Bookmark>>& bookmarks, Bookmark::BookmarkFilter filter)
{
	std::vector<std::shared_ptr<Bookmark>> result;

	if (filter == Bookmark::FILTER_ALL)
	{
		return bookmarks;
	}
	else if (filter == Bookmark::FILTER_NODES)
	{
		for (const std::shared_ptr<Bookmark>& bookmark: bookmarks)
		{
			if (std::dynamic_pointer_cast<NodeBookmark>(bookmark))
			{
				result.push_back(bookmark);
			}
		}
	}
	else if (filter == Bookmark::FILTER_EDGES)
	{
		for (const std::shared_ptr<Bookmark>& bookmark: bookmarks)
		{
			if (std::dynamic_pointer_cast<EdgeBookmark>(bookmark))
			{
				result.push_back(bookmark);
			}
		}
	}

	return result;
}

std::vector<std::shared_ptr<Bookmark>> BookmarkController::getOrderedBookmarks(
	const std::vector<std::shared_ptr<Bookmark>>& bookmarks, Bookmark::BookmarkOrder order)
{
	std::vector<std::shared_ptr<Bookmark>> result = bookmarks;

	if (order == Bookmark::ORDER_DATE_ASCENDING)
	{
		return getDateOrderedBookmarks(result, true);
	}
	else if (order == Bookmark::ORDER_DATE_DESCENDING)
	{
		return getDateOrderedBookmarks(result, false);
	}
	else if (order == Bookmark::ORDER_NAME_ASCENDING)
	{
		return getNameOrderedBookmarks(result, true);
	}
	else if (order == Bookmark::ORDER_NAME_DESCENDING)
	{
		return getNameOrderedBookmarks(result, false);
	}

	return result;
}

std::vector<std::shared_ptr<Bookmark>> BookmarkController::getDateOrderedBookmarks(
	const std::vector<std::shared_ptr<Bookmark>>& bookmarks, const bool ascending)
{
	std::vector<std::shared_ptr<Bookmark>> result = bookmarks;

	std::sort(result.begin(), result.end(), BookmarkController::bookmarkDateCompare);

	if (!ascending)
	{
		std::reverse(result.begin(), result.end());
	}

	return result;
}

std::vector<std::shared_ptr<Bookmark>> BookmarkController::getNameOrderedBookmarks(
	const std::vector<std::shared_ptr<Bookmark>>& bookmarks, const bool ascending)
{
	std::vector<std::shared_ptr<Bookmark>> result = bookmarks;

	std::sort(result.begin(), result.end(), BookmarkController::bookmarkNameCompare);

	if (!ascending)
	{
		std::reverse(result.begin(), result.end());
	}

	return result;
}

void BookmarkController::cleanBookmarkCategories()
{
	m_bookmarkCache.clear();

	std::vector<std::shared_ptr<Bookmark>> bookmarks = getAllBookmarks();

	for (const BookmarkCategory& category: getAllBookmarkCategories())
	{
		bool used = false;

		for (unsigned int j = 0; j < bookmarks.size(); j++)
		{
			if (bookmarks[j]->getCategory().getName() == category.getName())
			{
				used = true;
				break;
			}
		}

		if (!used)
		{
			m_storageAccess->removeBookmarkCategory(category.getId());
		}
	}
}

bool BookmarkController::bookmarkDateCompare(const std::shared_ptr<Bookmark> a, const std::shared_ptr<Bookmark> b)
{
	return a->getTimeStamp() < b->getTimeStamp();
}

bool BookmarkController::bookmarkNameCompare(const std::shared_ptr<Bookmark> a, const std::shared_ptr<Bookmark> b)
{
	return toLowerCase(a->getName()) < toLowerCase(b->getName());
}

void BookmarkController::update()
{
	BookmarkView* view = getView<BookmarkView>();
	if (view->bookmarkBrowserIsVisible())
	{
		view->displayBookmarkBrowser(getBookmarks(m_filter, m_order));
	}

	std::vector<std::shared_ptr<Bookmark>> bookmarks = getBookmarks(
		Bookmark::FILTER_ALL, Bookmark::ORDER_DATE_DESCENDING);

	const size_t maxBookmarkMenuCount = 20;
	if (bookmarks.size() > maxBookmarkMenuCount)
	{
		bookmarks.resize(maxBookmarkMenuCount);
	}

	Application::getInstance()->updateBookmarks(bookmarks);
}
