#ifndef MESSAGE_H
#define MESSAGE_H

#include <memory>
#include <string>

#include "MessageBase.h"
#include "MessageQueue.h"

template <typename MessageType>
class Message: public MessageBase
{
public:
	~Message() override = default;

	std::string getType() const override
	{
		return MessageType::getStaticType();
	}

	void dispatch() override
	{
		std::shared_ptr<MessageBase> message = std::make_shared<MessageType>(
			*dynamic_cast<MessageType*>(this));
		MessageQueue::getInstance()->pushMessage(message);
	}

	virtual void dispatchImmediately()
	{
		std::shared_ptr<MessageBase> message = std::make_shared<MessageType>(
			*dynamic_cast<MessageType*>(this));
		MessageQueue::getInstance()->processMessage(message, true);
	}

	void print(std::ostream&  /*os*/) const override {}
};

#endif	  // MESSAGE_H
