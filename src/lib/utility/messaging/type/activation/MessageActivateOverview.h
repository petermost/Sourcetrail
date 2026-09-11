#ifndef MESSAGE_ACTIVATE_ALL_H
#define MESSAGE_ACTIVATE_ALL_H

#include "Message.h"
#include "MessageActivateBase.h"

#include "NodeTypeSet.h"
#include "TabIds.h"

class MessageActivateOverview
	: public Message<MessageActivateOverview>
	, public MessageActivateBase
{
public:
	MessageActivateOverview(NodeTypeSet acceptedNodeTypes = NodeTypeSet::all())
		: acceptedNodeTypes(acceptedNodeTypes)
	{
		setIsParallel(true);
		setSchedulerId(TabIds::currentTab());
	}

	std::vector<SearchMatch> getSearchMatches() const override
	{
		if (acceptedNodeTypes != NodeTypeSet::all())
		{
			return SearchMatch::createCommandsForNodeTypes(acceptedNodeTypes);
		}
		return {SearchMatch::createCommand(SearchMatch::COMMAND_ALL)};
	}

	const NodeTypeSet acceptedNodeTypes;
};

#endif	  // MESSAGE_ACTIVATE_ALL_H
