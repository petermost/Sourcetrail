#include "QtBookmarkView.h"

#include "QtBookmarkBrowser.h"
#include "QtBookmarkCreator.h"
#include "QtMainView.h"
#include "QtMainWindow.h"
#include "TabId.h"

using namespace std;

QtBookmarkView::QtBookmarkView(ViewLayout* viewLayout)
	: BookmarkView(viewLayout), m_controllerProxy(this, TabId::app()) 
{
}

void QtBookmarkView::createWidgetWrapper() 
{
}

void QtBookmarkView::refreshView() 
{
}

void QtBookmarkView::displayBookmarkCreator(const std::vector<std::wstring>& names, const std::vector<BookmarkCategory>& categories, Id nodeId)
{
	m_onQtThread([=, this]() 
	{
		QtBookmarkCreator* bookmarkCreator = new QtBookmarkCreator(&m_controllerProxy, getMainWindow());
		bookmarkCreator->setupBookmarkCreator();

		std::wstring displayName;

		for (unsigned int i = 0; i < names.size(); i++)
		{
			displayName += names[i];

			if (i < names.size() - 1)
			{
				displayName += L"; ";
			}
		}

		bookmarkCreator->setDisplayName(displayName);
		bookmarkCreator->setBookmarkCategories(categories);
		bookmarkCreator->setNodeId(nodeId);

		bookmarkCreator->show();
		bookmarkCreator->raise();
	});
}

void QtBookmarkView::displayBookmarkCreator(std::shared_ptr<Bookmark> bookmark, const std::vector<BookmarkCategory>& categories)
{
	m_onQtThread([=, this]() 
	{
		QtBookmarkCreator* bookmarkCreator = new QtBookmarkCreator(&m_controllerProxy, getMainWindow(), bookmark->getId());

		bookmarkCreator->setupBookmarkCreator();
		bookmarkCreator->setDisplayName(bookmark->getName());
		bookmarkCreator->setComment(bookmark->getComment());
		bookmarkCreator->setBookmarkCategories(categories);
		bookmarkCreator->setCurrentBookmarkCategory(bookmark->getCategory());

		bookmarkCreator->show();
		bookmarkCreator->raise();
	});
}

void QtBookmarkView::displayBookmarkBrowser(const std::vector<std::shared_ptr<Bookmark>>& bookmarks)
{
	m_onQtThread([=, this]() 
	{
		if (m_bookmarkBrowser == nullptr)
		{
			m_bookmarkBrowser = new QtBookmarkBrowser(&m_controllerProxy, getMainWindow());
			m_bookmarkBrowser->setupBookmarkBrowser();
		}
		m_bookmarkBrowser->setBookmarks(bookmarks);
		m_bookmarkBrowser->show();
		m_bookmarkBrowser->raise();
	});
}

void QtBookmarkView::undisplayBookmarkBrowser()
{
	m_onQtThread([=, this]() 
	{
		if (m_bookmarkBrowser != nullptr)
			m_bookmarkBrowser->hide();
	});
}

bool QtBookmarkView::bookmarkBrowserIsVisible() const
{
	if (m_bookmarkBrowser != nullptr)
	{
		return m_bookmarkBrowser->isVisible();
	}
	else
	{
		return false;
	}
}

inline QtMainWindow *QtBookmarkView::getMainWindow() const
{
	return dynamic_cast<QtMainView*>(dynamic_cast<View*>(getViewLayout())->getViewLayout())->getMainWindow();
}
