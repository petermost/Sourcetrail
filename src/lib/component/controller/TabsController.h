#ifndef TABS_CONTROLLER_H
#define TABS_CONTROLLER_H

#include "MessageActivateErrors.h"
#include "MessageIndexingFinished.h"
#include "MessageListener.h"
#include "MessageTabClose.h"
#include "MessageTabOpen.h"
#include "MessageTabOpenWith.h"
#include "MessageTabSelect.h"
#include "MessageTabState.h"

#include "Controller.h"
#include "Tab.h"
#include "TabsView.h"

#include <aidkit/thread_shared.hpp>

struct SearchMatch;

class StorageAccess;
class ViewFactory;
class ViewLayout;

class TabsController
	: public Controller
	, public MessageListener<MessageActivateErrors>
	, public MessageListener<MessageIndexingFinished>
	, public MessageListener<MessageTabClose>
	, public MessageListener<MessageTabOpen>
	, public MessageListener<MessageTabOpenWith>
	, public MessageListener<MessageTabSelect>
	, public MessageListener<MessageTabState>
{
public:
	TabsController(
		ViewLayout* mainLayout,
		const ViewFactory* viewFactory,
		StorageAccess* storageAccess,
		ScreenSearchSender* screenSearchSender);

	// Controller implementation
	void clear() override;
	
	void addTab(TabId tabId, const SearchMatch &match);
	void showTab(TabId tabId);
	void removeTab(TabId tabId);
	void destroyTab(TabId tabId);
	void onClearTabs();

private:
	void handleMessage(MessageActivateErrors* message) override;
	void handleMessage(MessageIndexingFinished* message) override;
	void handleMessage(MessageTabClose* message) override;
	void handleMessage(MessageTabOpen* message) override;
	void handleMessage(MessageTabOpenWith* message) override;
	void handleMessage(MessageTabSelect* message) override;
	void handleMessage(MessageTabState* message) override;

	TabsView* getView() const;

	ViewLayout *const m_mainLayout;
	const ViewFactory *const m_viewFactory;
	StorageAccess *const m_storageAccess;
	ScreenSearchSender *const m_screenSearchSender;

	aidkit::thread_shared<std::map<TabId, std::shared_ptr<Tab>>> m_tabs;
	std::atomic<bool> m_isCreatingTab = false;
	aidkit::thread_shared<std::tuple<Id, FilePath, size_t>> m_scrollToLine;
};

#endif	  // TABS_CONTROLLER_H
