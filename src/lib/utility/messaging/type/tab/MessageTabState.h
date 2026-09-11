#ifndef MESSAGE_TAB_STATE_H
#define MESSAGE_TAB_STATE_H

#include "Message.h"
#include "SearchMatch.h"

class MessageTabState: public Message<MessageTabState>
{
public:
	MessageTabState(TabId tabId, const std::vector<SearchMatch>& searchMatches)
		: tabId(tabId), searchMatches(searchMatches)
	{
	}

	const TabId tabId;
	const std::vector<SearchMatch> searchMatches;
};

#endif	  // MESSAGE_TAB_STATE_H
