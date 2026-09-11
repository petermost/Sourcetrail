#ifndef MESSAGE_H
#define MESSAGE_H

#include "MessageBase.h"
#include "MessageQueue.h"

#include <memory>

template <typename MessageType>
class Message : public MessageBase
{
public:
	~Message() override = default;

	const std::type_info &getType() const final
	{
		return typeid(MessageType);
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
