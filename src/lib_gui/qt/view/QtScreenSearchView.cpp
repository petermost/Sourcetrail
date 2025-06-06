#include "QtScreenSearchView.h"

#include "QtMainView.h"
#include "QtMainWindow.h"
#include "QtScreenSearchBox.h"
#include "QtViewWidgetWrapper.h"
#include "TabIds.h"
#include "utilityQt.h"
#include "QtResources.h"

#include <QToolBar>

using namespace utility;

QtScreenSearchView::QtScreenSearchView(ViewLayout* viewLayout)
	: ScreenSearchView(viewLayout), m_controllerProxy(this, TabIds::app())
{
	m_widget = new QtScreenSearchBox(&m_controllerProxy);

	m_bar = new QToolBar();
	m_bar->setObjectName(QStringLiteral("screen_search_bar"));
	m_bar->setMovable(false);
	m_bar->addWidget(m_widget);

	QObject::connect(m_widget, &QtScreenSearchBox::closePressed, this, &QtScreenSearchView::hide);

	if (QtMainWindow* mainWindow = utility::getMainWindowforMainView(getViewLayout()))
	{
		QObject::connect(
			mainWindow, &QtMainWindow::showScreenSearch, this, &QtScreenSearchView::show);
		QObject::connect(
			mainWindow, &QtMainWindow::hideScreenSearch, this, &QtScreenSearchView::hide);
	}
}

void QtScreenSearchView::createWidgetWrapper()
{
	setWidgetWrapper(std::make_shared<QtViewWidgetWrapper>(m_widget));
}

void QtScreenSearchView::refreshView()
{
	m_onQtThread([=, this]()
	{
		m_bar->setStyleSheet(QtResources::loadStyleSheet(QtResources::SCREEN_SEARCH_VIEW_CSS));
	});
}

void QtScreenSearchView::setMatchCount(size_t matchCount)
{
	m_onQtThread([=, this]() { m_widget->setMatchCount(matchCount); });
}

void QtScreenSearchView::setMatchIndex(size_t matchIndex)
{
	m_onQtThread([=, this]() { m_widget->setMatchIndex(matchIndex); });
}

void QtScreenSearchView::addResponder(const std::string& name)
{
	m_widget->addResponder(name);
}

void QtScreenSearchView::show()
{
	if (QtMainWindow* mainWindow = utility::getMainWindowforMainView(getViewLayout()))
	{
		mainWindow->addToolBar(Qt::BottomToolBarArea, m_bar);
	}

	m_bar->show();
	m_widget->setFocus();
}

void QtScreenSearchView::hide()
{
	m_bar->hide();
	m_widget->setMatchCount(0);

	m_controllerProxy.executeAsTask(&ScreenSearchController::clearMatches);
}
