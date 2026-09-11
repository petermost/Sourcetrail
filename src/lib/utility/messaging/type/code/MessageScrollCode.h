#ifndef MESSAGE_SCROLL_CODE_H
#define MESSAGE_SCROLL_CODE_H

#include "Message.h"
#include "TabIds.h"

class MessageScrollCode: public Message<MessageScrollCode>
{
public:
	MessageScrollCode(int value, bool inListMode): value(value), inListMode(inListMode)
	{
		setIsLogged(false);
		setSchedulerId(TabIds::currentTab());
	}

	int value;
	const bool inListMode;
};

#endif	  // MESSAGE_SCROLL_CODE_H
