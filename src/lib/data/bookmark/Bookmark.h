#ifndef BOOKMARK_H
#define BOOKMARK_H

#include <string>

#include "TimeStamp.h"
#include "BookmarkCategory.h"

enum class BookmarkId : std::size_t
{
	NONE = 0
};

class Bookmark
{
public:
	enum BookmarkFilter
	{
		FILTER_UNKNOWN = 0,
		FILTER_ALL,
		FILTER_NODES,
		FILTER_EDGES
	};

	enum BookmarkOrder
	{
		ORDER_NONE = 0,
		ORDER_DATE_ASCENDING,
		ORDER_DATE_DESCENDING,
		ORDER_NAME_ASCENDING,
		ORDER_NAME_DESCENDING
	};

	Bookmark(
		const BookmarkId bookmarkId,
		const std::string& name,
		const std::string& comment,
		const TimeStamp& timeStamp,
		const BookmarkCategory& category);

	virtual ~Bookmark() = default;
	
	void setIsValid(const bool isValid = true);
	bool isValid() const;
	
	BookmarkId getId() const;
	std::string getName() const;
	std::string getComment() const;
	TimeStamp getTimeStamp() const;
	BookmarkCategory getCategory() const;

private:
	bool m_isValid = false;
	
	const BookmarkId m_bookmarkId;
	const std::string m_name;
	const std::string m_comment;
	const TimeStamp m_timeStamp;
	const BookmarkCategory m_category;
};

#endif	  // BOOKMARK_H
