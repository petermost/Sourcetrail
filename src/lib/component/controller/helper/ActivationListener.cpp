#include "ActivationListener.h"

const std::vector<SearchMatch>& ActivationListener::getSearchMatches() const
{
	return m_searchMatches;
}

void ActivationListener::handleMessage(MessageActivateErrors* message)
{
	handleMessageActivateBase(message);
}

void ActivationListener::handleMessage(MessageActivateFullTextSearch* message)
{
	handleMessageActivateBase(message);
}

void ActivationListener::handleMessage(MessageActivateLegend* message)
{
	handleMessageActivateBase(message);
}

void ActivationListener::handleMessage(MessageActivateOverview* message)
{
	handleMessageActivateBase(message);
}

void ActivationListener::handleMessage(MessageActivateTokens* message)
{
	handleMessageActivateBase(message);
}

void ActivationListener::handleMessage(MessageActivateTrail* message)
{
	handleMessageActivateBase(message);
}

void ActivationListener::handleMessageActivateBase(const MessageActivateBase* message)
{
	m_searchMatches = message->getSearchMatches();
	handleActivation(message);
	handleActivation(m_searchMatches);
}

void ActivationListener::handleActivation(const MessageActivateBase*  /*message*/) {}

void ActivationListener::handleActivation(const std::vector<SearchMatch>&  /*searchMatches*/) {}
