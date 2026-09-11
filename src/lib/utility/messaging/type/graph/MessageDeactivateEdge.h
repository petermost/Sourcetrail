#ifndef MESSAGE_DEACTIVATE_EDGE_H
#define MESSAGE_DEACTIVATE_EDGE_H

#include "Message.h"
#include "TabIds.h"

class MessageDeactivateEdge: public Message<MessageDeactivateEdge>
{
public:
	MessageDeactivateEdge(bool scrollToDefinition): scrollToDefinition(scrollToDefinition)
	{
		setSchedulerId(TabIds::currentTab());
	}

	const bool scrollToDefinition;
};

#endif	  // MESSAGE_DEACTIVATE_EDGE_H
