#include "MessageListenerBase.h"
#include "MessageQueue.h"

MessageListenerBase::MessageListenerBase()
{
	MessageQueue::getInstance()->registerListener(this);
}

MessageListenerBase::~MessageListenerBase()
{
	MessageQueue::getInstance()->unregisterListener(this);
}

const std::type_info &MessageListenerBase::getType() const
{
	return typeid(MessageListenerBase);
}

void MessageListenerBase::handleMessageBase(MessageBase * /* message */)
{
}

TabId MessageListenerBase::getSchedulerId() const
{
	return TabId::NONE;
}
