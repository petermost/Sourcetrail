#ifndef BOOKMARK_VIEW_H
#define BOOKMARK_VIEW_H

#include "Bookmark.h"
#include "View.h"

class BookmarkController;

class BookmarkView: public View
{
public:
	BookmarkView(ViewLayout* viewLayout);
	~BookmarkView() override = default;

	std::string getName() const override;

	virtual void displayBookmarkBrowser(const std::vector<std::shared_ptr<Bookmark>>& bookmarks) = 0;
	virtual void displayBookmarkCreator(std::shared_ptr<Bookmark> bookmark, const std::vector<BookmarkCategory>& categories) = 0;
	virtual void displayBookmarkCreator(const std::vector<std::string>& names, const std::vector<BookmarkCategory>& categories, Id nodeId) = 0;

	virtual void undisplayBookmarkBrowser() = 0;
	
	virtual bool bookmarkBrowserIsVisible() const = 0;

private:
	BookmarkController* getController();
};

#endif	  // BOOKMARK_VIEW_H
