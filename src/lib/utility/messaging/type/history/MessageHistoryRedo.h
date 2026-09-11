#ifndef MESSAGE_HISTORY_REDO_H
#define MESSAGE_HISTORY_REDO_H

#include "Message.h"
#include "TabIds.h"

class MessageHistoryRedo: public Message<MessageHistoryRedo>
{
public:
	MessageHistoryRedo()
	{
		setSchedulerId(TabIds::currentTab());
	}
};

#endif	  // MESSAGE_HISTORY_REDO_H
