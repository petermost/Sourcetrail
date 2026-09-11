#ifndef MESSAGE_QUIT_APPLICATION_H
#define MESSAGE_QUIT_APPLICATION_H

#include "Message.h"

class MessageQuitApplication: public Message<MessageQuitApplication>
{
public:
	MessageQuitApplication() = default;
};

#endif	  // MESSAGE_QUIT_APPLICATION_H
