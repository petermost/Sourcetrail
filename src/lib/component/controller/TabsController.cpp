#include "TabsController.h"

#include "Application.h"
#include "MessageFind.h"
#include "MessageIndexingFinished.h"
#include "MessageScrollToLine.h"
#include "MessageSearch.h"
#include "MessageWindowChanged.h"
#include "ScreenSearchInterfaces.h"
#include "TabIds.h"
#include "TaskLambda.h"
#include "TaskManager.h"
#include "TaskScheduler.h"
#include "utilityStl.h"

TabsController::TabsController(ViewLayout *mainLayout, const ViewFactory *viewFactory, StorageAccess *storageAccess,
	ScreenSearchSender *screenSearchSender)
	: m_mainLayout(mainLayout)
	, m_viewFactory(viewFactory)
	, m_storageAccess(storageAccess)
	, m_screenSearchSender(screenSearchSender)
{
}

void TabsController::clear()
{
	getView()->clear();
	m_isCreatingTab = false;

	while (true)
	{
		if (m_tabs.access()->empty())
		{
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void TabsController::addTab(TabId tabId, const SearchMatch &match)
{
	TaskManager::createScheduler(tabId)->startSchedulerLoopThreaded();

	m_tabs.access()->emplace(tabId, std::make_shared<Tab>(tabId, m_viewFactory, m_storageAccess, m_screenSearchSender));

	MessageWindowChanged().dispatch();

	if (match.isValid())
	{
		MessageSearch msg({match}, NodeTypeSet::all());
		msg.setSchedulerId(tabId);
		msg.dispatch();
		aidkit::access([tabId, &match](auto &scrollToLine)
		{
			if (match.tokenIds.size() && std::get<0>(scrollToLine) == match.tokenIds[0])
			{
				MessageScrollToLine scrollMsg(std::get<1>(scrollToLine), std::get<2>(scrollToLine));
				scrollMsg.setSchedulerId(tabId);
				scrollMsg.dispatch();
			}
		}, m_scrollToLine);
	}
	else
	{
		MessageFind msg;
		msg.setSchedulerId(tabId);
		msg.dispatch();
	}

	m_scrollToLine = std::make_tuple(0, FilePath(), 0);
	m_isCreatingTab = false;
}

void TabsController::showTab(TabId tabId)
{
	auto optIt = utility::find_optional(*m_tabs.access(), tabId);
	if (optIt)
	{
		TabIds::setCurrentTabId(tabId);
		(*optIt)->second->setParentLayout(m_mainLayout);
	}
	else
	{
		TabIds::setCurrentTabId(TabId::NONE);
		m_mainLayout->showOriginalViews();
	}

	Task::dispatch(TabIds::app(), std::make_shared<TaskLambda>([this]()
	{
		m_screenSearchSender->clearMatches();
	}));
}

void TabsController::removeTab(TabId tabId)
{
	// use app task scheduler thread to stop running tasks of tab
	Task::dispatch(TabIds::background(), std::make_shared<TaskLambda>([tabId, this]()
	{
		m_screenSearchSender->clearMatches();

		TaskScheduler *scheduler = TaskManager::getScheduler(tabId).get();
		scheduler->terminateRunningTasks();
		scheduler->stopSchedulerLoop();

		TaskManager::destroyScheduler(tabId);

		getView()->destroyTab(tabId);
	}));
}

void TabsController::destroyTab(TabId tabId)
{
	aidkit::access([this, tabId](auto &tabs)
	{
		// destroy the tab on the qt thread to allow view destruction
		tabs.erase(tabId);

		if (tabs.empty() && Application::getInstance()->isProjectLoaded() && !m_isCreatingTab)
		{
			MessageTabOpen().dispatch();
			m_isCreatingTab = true;
		}
	}, m_tabs);
}

void TabsController::onClearTabs()
{
	TabIds::setCurrentTabId(TabId::NONE);
	m_mainLayout->showOriginalViews();
}

TabsView* TabsController::getView() const
{
	return Controller::getView<TabsView>();
}

void TabsController::handleMessage(MessageActivateErrors*  /*message*/)
{
	if (m_tabs.access()->empty() && Application::getInstance()->isProjectLoaded())
	{
		MessageTabOpenWith(SearchMatch::createCommand(SearchMatch::COMMAND_ERROR)).dispatch();
	}
}

void TabsController::handleMessage(MessageIndexingFinished*  /*message*/)
{
	if (m_tabs.access()->empty() && Application::getInstance()->isProjectLoaded())
	{
		MessageTabOpenWith(SearchMatch::createCommand(SearchMatch::COMMAND_ALL)).dispatch();
	}
}

void TabsController::handleMessage(MessageTabClose*  /*message*/)
{
	getView()->closeTab();
}

void TabsController::handleMessage(MessageTabOpen*  /*message*/)
{
	if (Application::getInstance()->isProjectLoaded())
	{
		getView()->openTab(true, SearchMatch());
		m_isCreatingTab = true;
	}
}

void TabsController::handleMessage(MessageTabOpenWith* message)
{
	if (!Application::getInstance()->isProjectLoaded())
	{
		return;
	}

	SearchMatch match = message->match;
	if (!match.isValid())
	{
		Id tokenId = message->tokenId;
		if (!tokenId && message->locationId)
		{
			std::vector<Id> tokenIds = m_storageAccess->getNodeIdsForLocationIds({message->locationId});
			if (tokenIds.size())
			{
				tokenId = tokenIds[0];
			}
		}

		if (!tokenId && !message->filePath.empty())
		{
			tokenId = m_storageAccess->getNodeIdForFileNode(message->filePath);

			if (message->line)
			{
				m_scrollToLine = std::make_tuple(tokenId, message->filePath, message->line);
			}
		}

		if (tokenId)
		{
			std::vector<SearchMatch> matches = m_storageAccess->getSearchMatchesForTokenIds({tokenId});
			if (matches.size())
			{
				match = matches[0];
			}
		}
	}

	if (match.isValid())
	{
		getView()->openTab(message->showTab, match);
		m_isCreatingTab = true;
	}
}

void TabsController::handleMessage(MessageTabSelect* message)
{
	getView()->selectTab(message->next);
}

void TabsController::handleMessage(MessageTabState* message)
{
	getView()->updateTab(message->tabId, message->searchMatches);
}
