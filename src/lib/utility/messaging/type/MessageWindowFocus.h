#ifndef MESSAGE_WINDOW_FOCUS_H
#define MESSAGE_WINDOW_FOCUS_H

#include "Message.h"

class MessageWindowFocus: public Message<MessageWindowFocus>
{
public:
	MessageWindowFocus(bool focusIn): focusIn(focusIn) {}

	const bool focusIn;
};

#endif	  // MESSAGE_WINDOW_FOCUS_H
