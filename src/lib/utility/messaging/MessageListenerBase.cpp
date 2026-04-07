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

TabId MessageListenerBase::getSchedulerId() const
{
	return TabId::NONE;
}
