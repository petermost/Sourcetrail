#ifndef EDGE_BOOKMARK_H
#define EDGE_BOOKMARK_H

#include "Bookmark.h"

class EdgeBookmark: public Bookmark
{
public:
	EdgeBookmark(
		const BookmarkId bookmarkId,
		const std::string& name,
		const std::string& comment,
		const TimeStamp& timeStamp,
		const BookmarkCategory& category);

	void addEdgeId(const Id edgeId);
	void setEdgeIds(const std::vector<Id>& edgesIds);
	std::vector<Id> getEdgeIds() const;

	void setActiveNodeId(const Id activeNodeId);
	Id getActiveNodeId() const;

private:
	std::vector<Id> m_edgeIds;
	Id m_activeNodeId;
};

#endif	  // EDGE_BOOKMARK_H
