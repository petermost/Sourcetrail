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

std::string MessageListenerBase::getType() const
{
	return "MessageListenerBase";
}

void MessageListenerBase::handleMessageBase(MessageBase * /* message */)
{
}

TabId MessageListenerBase::getSchedulerId() const
{
	return TabId::NONE;
}
