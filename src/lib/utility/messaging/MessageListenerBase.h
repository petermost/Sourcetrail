#ifndef MESSAGE_LISTENER_BASE_H
#define MESSAGE_LISTENER_BASE_H

#include "MessageBase.h"
#include "MessageQueue.h"

#include <string>

class MessageListenerBase
{
public:
	MessageListenerBase()
	{
		MessageQueue::getInstance()->registerListener(this);
	}

	virtual ~MessageListenerBase()
	{
		MessageQueue::getInstance()->unregisterListener(this);
	}

	Id getId() const
	{
		return m_id;
	}

	virtual std::string getType() const = 0;

	virtual void handleMessageBase(MessageBase* message) = 0;

	virtual TabId getSchedulerId() const
	{
		return TabId::NONE;
	}

private:
	static Id s_nextId;

	Id m_id = s_nextId++;
};

#endif	  // MESSAGE_LISTENER_BASE_H
