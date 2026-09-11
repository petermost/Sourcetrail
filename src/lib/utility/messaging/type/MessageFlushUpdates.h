#ifndef MESSAGE_FLUSH_UPDATES_H
#define MESSAGE_FLUSH_UPDATES_H

#include "Message.h"
#include "TabIds.h"

class MessageFlushUpdates: public Message<MessageFlushUpdates>
{
public:
	MessageFlushUpdates(bool keepsContent = false)
	{
		setKeepContent(keepsContent);
		setSchedulerId(TabIds::currentTab());
	}
};

#endif	  // MESSAGE_FLUSH_UPDATES_H
