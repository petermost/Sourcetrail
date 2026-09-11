#ifndef MESSAGE_ACTIVATE_WINDOW_H
#define MESSAGE_ACTIVATE_WINDOW_H

#include "Message.h"

class MessageActivateWindow: public Message<MessageActivateWindow>
{
public:
	MessageActivateWindow() = default;
};

#endif	  // MESSAGE_ACTIVATE_WINDOW_H
