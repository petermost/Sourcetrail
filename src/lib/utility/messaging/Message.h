#ifndef MESSAGE_H
#define MESSAGE_H

#include "MessageBase.h"
#include "MessageQueue.h"

#include <memory>
#include <string>

template <typename MessageType>
class Message : public MessageBase
{
public:
	~Message() override = default;

	std::string getType() const final
	{
		return MessageType::getStaticType();
	}

	void dispatch() final
	{
		std::shared_ptr message = std::make_shared<MessageType>(*static_cast<MessageType *>(this));

		MessageQueue::getInstance()->pushMessage(message);
	}

	void dispatchImmediately()
	{
		std::shared_ptr message = std::make_shared<MessageType>(*static_cast<MessageType *>(this));

		MessageQueue::getInstance()->processMessage(message, true);
	}
};

#endif // MESSAGE_H
