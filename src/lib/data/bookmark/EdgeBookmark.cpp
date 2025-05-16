#include "EdgeBookmark.h"

EdgeBookmark::EdgeBookmark(
	const BookmarkId bookmarkId,
	const std::string& name,
	const std::string& comment,
	const TimeStamp& timeStamp,
	const BookmarkCategory& category)
	: Bookmark(bookmarkId, name, comment, timeStamp, category)
{
}

void EdgeBookmark::addEdgeId(const Id edgeId)
{
	m_edgeIds.push_back(edgeId);
}

void EdgeBookmark::setEdgeIds(const std::vector<Id>& edgesIds)
{
	m_edgeIds = edgesIds;
}

std::vector<Id> EdgeBookmark::getEdgeIds() const
{
	return m_edgeIds;
}

void EdgeBookmark::setActiveNodeId(const Id activeNodeId)
{
	m_activeNodeId = activeNodeId;
}

Id EdgeBookmark::getActiveNodeId() const
{
	return m_activeNodeId;
}
