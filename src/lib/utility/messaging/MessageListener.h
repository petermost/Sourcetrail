#ifndef MESSAGE_LISTENER_H
#define MESSAGE_LISTENER_H

#include "MessageBase.h"
#include "MessageListenerBase.h"
#include "MessageQueue.h"

template <typename MessageType>
class MessageListener: public MessageListenerBase
{
public:
	MessageListener() = default;

private:
	const std::type_info &getType() const final
	{
		return typeid(MessageType);
	}

	void handleMessageBase(MessageBase* message) final
	{
		// if (message->isLogged())
		// {
		// 	LOG_INFO_STREAM_BARE(<< "handle " << message->str());
		// }

		handleMessage(static_cast<MessageType *>(message));
	}

	virtual void handleMessage(MessageType* message) = 0;
};

#endif	  // MESSAGE_LISTENER_H
