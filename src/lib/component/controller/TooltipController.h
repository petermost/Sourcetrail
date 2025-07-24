#ifndef TOOLTIP_CONTROLLER_H
#define TOOLTIP_CONTROLLER_H

#include "Controller.h"
#include "MessageActivateLocalSymbols.h"
#include "MessageActivateTokens.h"
#include "MessageFocusIn.h"
#include "MessageFocusOut.h"
#include "MessageGraphNodeExpand.h"
#include "MessageListener.h"
#include "MessageScrollCode.h"
#include "MessageScrollGraph.h"
#include "MessageTooltipHide.h"
#include "MessageTooltipShow.h"
#include "MessageWindowFocus.h"

#include <aidkit/thread_shared.hpp>

class StorageAccess;
class TooltipView;

class TooltipController
	: public Controller
	, public MessageListener<MessageActivateTokens>
	, public MessageListener<MessageActivateLocalSymbols>
	, public MessageListener<MessageFocusIn>
	, public MessageListener<MessageFocusOut>
	, public MessageListener<MessageGraphNodeExpand>
	, public MessageListener<MessageScrollCode>
	, public MessageListener<MessageScrollGraph>
	, public MessageListener<MessageTooltipHide>
	, public MessageListener<MessageTooltipShow>
	, public MessageListener<MessageWindowFocus>
{
public:
	TooltipController(StorageAccess* storageAccess);
	~TooltipController() override;

	// Controller
	void clear() override;

	// MessageListener
	void handleMessage(MessageActivateTokens* message) override;
	void handleMessage(MessageActivateLocalSymbols* message) override;
	void handleMessage(MessageFocusIn* message) override;
	void handleMessage(MessageFocusOut* message) override;
	void handleMessage(MessageGraphNodeExpand* message) override;
	void handleMessage(MessageScrollCode* message) override;
	void handleMessage(MessageScrollGraph* message) override;
	void handleMessage(MessageTooltipHide* message) override;
	void handleMessage(MessageTooltipShow* message) override;
	void handleMessage(MessageWindowFocus* message) override;

private:
	struct TooltipRequest
	{
		static aidkit::thread_shared<Id> s_requestId;

		Id requestId;
		std::vector<Id> tokenIds;

		TooltipInfo info;
		TooltipOrigin origin;
	};

	TooltipView* getView() const;
	View* getViewForOrigin(TooltipOrigin origin) const;

	void requestTooltipShow(const std::vector<Id> &tokenIds, const TooltipInfo &info, TooltipOrigin origin);
	void requestTooltipHide();

	StorageAccess *const m_storageAccess;

	aidkit::thread_shared<std::unique_ptr<TooltipRequest>> m_showRequest;
	std::atomic<bool> m_hideRequest = false;
};

#endif	  // TOOLTIP_CONTROLLER_H
