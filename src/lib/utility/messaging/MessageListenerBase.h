#ifndef MESSAGE_LISTENER_BASE_H
#define MESSAGE_LISTENER_BASE_H

#include "MessageBase.h"

class MessageListenerBase
{
public:
	MessageListenerBase();
	virtual ~MessageListenerBase();

	virtual const std::type_info &getType() const;

	virtual void handleMessageBase(MessageBase* message);

	virtual TabId getSchedulerId() const;
};

#endif	  // MESSAGE_LISTENER_BASE_H
