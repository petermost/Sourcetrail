#ifndef MESSAGE_HISTORY_TO_POSITION_H
#define MESSAGE_HISTORY_TO_POSITION_H

#include "Message.h"
#include "TabIds.h"

class MessageHistoryToPosition: public Message<MessageHistoryToPosition>
{
public:
	MessageHistoryToPosition(size_t index): index(index)
	{
		setSchedulerId(TabIds::currentTab());
	}

	const size_t index;
};

#endif	  // MESSAGE_HISTORY_TO_POSITION_H
