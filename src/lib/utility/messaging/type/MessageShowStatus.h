#ifndef MESSAGE_SHOW_STATUS_H
#define MESSAGE_SHOW_STATUS_H

#include "Message.h"

class MessageShowStatus: public Message<MessageShowStatus>
{
public:
	MessageShowStatus() = default;
};

#endif	  // MESSAGE_SHOW_STATUS_H
