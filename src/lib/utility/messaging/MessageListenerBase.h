#ifndef MESSAGE_LISTENER_BASE_H
#define MESSAGE_LISTENER_BASE_H

#include "MessageBase.h"

#include <string>

class MessageListenerBase
{
public:
	MessageListenerBase();
	virtual ~MessageListenerBase();

	virtual std::string getType() const = 0;

	virtual void handleMessageBase(MessageBase* message) = 0;

	virtual TabId getSchedulerId() const;
};

#endif	  // MESSAGE_LISTENER_BASE_H
