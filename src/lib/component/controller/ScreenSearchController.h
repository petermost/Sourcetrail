#ifndef SCREEN_SEARCH_CONTROLLER_H
#define SCREEN_SEARCH_CONTROLLER_H

#include "ActivationListener.h"
#include "Controller.h"
#include "MessageActivateLocalSymbols.h"
#include "MessageActivateTrailEdge.h"
#include "MessageChangeFileView.h"
#include "MessageCodeShowDefinition.h"
#include "MessageDeactivateEdge.h"
#include "MessageGraphNodeBundleSplit.h"
#include "MessageGraphNodeExpand.h"
#include "MessageGraphNodeHide.h"
#include "MessageListener.h"
#include "ScreenSearchInterfaces.h"

#include <aidkit/thread_shared.hpp>

#include <set>

class ScreenSearchController
	: public Controller
	, public ScreenSearchSender
	, public ActivationListener
	, public MessageListener<MessageActivateLocalSymbols>
	, public MessageListener<MessageActivateTrailEdge>
	, public MessageListener<MessageChangeFileView>
	, public MessageListener<MessageCodeShowDefinition>
	, public MessageListener<MessageDeactivateEdge>
	, public MessageListener<MessageGraphNodeBundleSplit>
	, public MessageListener<MessageGraphNodeExpand>
	, public MessageListener<MessageGraphNodeHide>
{
public:
	~ScreenSearchController() override = default;

	// Controller implementation
	void clear() override;

	// ScreenSearchSender implementation
	void foundMatches(ScreenSearchResponder* responder, size_t matchCount) override;
	void addResponder(ScreenSearchResponder* responder) override;
	void removeResponder(ScreenSearchResponder* responder) override;
	void clearMatches() override;

	void search(const std::string& query, const std::set<std::string>& responderNames);
	void activateMatch(bool next);

private:
	size_t getResponderId(ScreenSearchResponder* responder) const;

	void handleActivation(const MessageActivateBase* message) override;

	void handleMessage(MessageActivateLocalSymbols* message) override;
	void handleMessage(MessageActivateTrailEdge* message) override;
	void handleMessage(MessageChangeFileView* message) override;
	void handleMessage(MessageCodeShowDefinition* message) override;
	void handleMessage(MessageDeactivateEdge* message) override;
	void handleMessage(MessageGraphNodeBundleSplit* message) override;
	void handleMessage(MessageGraphNodeExpand* message) override;
	void handleMessage(MessageGraphNodeHide* message) override;

	aidkit::thread_shared<std::vector<ScreenSearchResponder *>> m_responders;
	aidkit::thread_shared<std::vector<std::pair<size_t, size_t>>> m_matches;
	std::atomic<size_t> m_matchIndex = 0;
};

#endif	  // SCREEN_SEARCH_CONTROLLER_H
