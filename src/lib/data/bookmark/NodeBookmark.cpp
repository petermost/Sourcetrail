#include "NodeBookmark.h"

NodeBookmark::NodeBookmark(
	const BookmarkId bookmarkId,
	const std::string& name,
	const std::string& comment,
	const TimeStamp& timeStamp,
	const BookmarkCategory& category)
	: Bookmark(bookmarkId, name, comment, timeStamp, category)
{
}

void NodeBookmark::addNodeId(const Id nodeId)
{
	m_nodeIds.push_back(nodeId);
}

void NodeBookmark::setNodeIds(const std::vector<Id>& nodeIds)
{
	m_nodeIds = nodeIds;
}

std::vector<Id> NodeBookmark::getNodeIds() const
{
	return m_nodeIds;
}
