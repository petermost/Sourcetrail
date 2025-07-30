#include "MessageQueue.h"

#include "MessageBase.h"
#include "MessageFilter.h"
#include "MessageListenerBase.h"
#include "TabIds.h"
#include "TaskGroupParallel.h"
#include "TaskGroupSequence.h"
#include "TaskLambda.h"
#include "logging.h"

#include <chrono>
#include <thread>

std::shared_ptr<MessageQueue> MessageQueue::s_instance;

std::shared_ptr<MessageQueue> MessageQueue::getInstance()
{
	if (!s_instance)
	{
		s_instance = std::shared_ptr<MessageQueue>(new MessageQueue());
	}
	return s_instance;
}

MessageQueue::~MessageQueue()
{
	aidkit::access([](auto &listeners)
	{
		for (size_t i = 0; i < listeners.size(); i++)
		{
			listeners[i]->removedListener();
		}
		listeners.clear();
	}, m_listeners);
}

void MessageQueue::registerListener(MessageListenerBase* listener)
{
	m_listeners.access()->push_back(listener);
}

void MessageQueue::unregisterListener(MessageListenerBase* listener)
{
	aidkit::access([this, listener](auto &listeners)
	{
		for (size_t i = 0; i < listeners.size(); i++)
		{
			if (listeners[i] == listener)
			{
				listeners.erase(listeners.begin() + i);

				// m_currentListenerIndex and m_listenersLength need to be updated in case this happens
				// while a message is handled.
				if (i <= m_currentListenerIndex)
				{
					m_currentListenerIndex--;
				}

				if (i < m_listenersLength)
				{
					m_listenersLength--;
				}

				return;
			}
		}

		LOG_ERROR("Listener was not found");
	}, m_listeners);
}

MessageListenerBase* MessageQueue::getListenerById(Id listenerId) const
{
	return aidkit::access([listenerId](auto &listeners)
	{
		for (size_t i = 0; i < listeners.size(); i++)
		{
			if (listeners[i]->getId() == listenerId)
			{
				return listeners[i];
			}
		}
		return (MessageListenerBase *)nullptr;
	}, m_listeners);
}

void MessageQueue::addMessageFilter(std::shared_ptr<MessageFilter> filter)
{
	m_filters.access()->push_back(filter);
}

void MessageQueue::pushMessage(std::shared_ptr<MessageBase> message)
{
	m_messageBuffer.access()->push_back(message);
}

void MessageQueue::processMessage(std::shared_ptr<MessageBase> message, bool asNextTask)
{
	if (message->isLogged())
	{
		LOG_INFO_BARE("send " + message->str());
	}

	if (m_sendMessagesAsTasks && message->sendAsTask())
	{
		sendMessageAsTask(message, asNextTask);
	}
	else
	{
		sendMessage(message);
	}
}

void MessageQueue::startMessageLoopThreaded()
{
	std::thread(&MessageQueue::startMessageLoop, this).detach();

	m_threadIsRunning = true;
}

void MessageQueue::startMessageLoop()
{
	if (m_loopIsRunning)
	{
		LOG_ERROR("Loop is already running");
		return;
	}

	m_loopIsRunning = true;

	while (true)
	{
		processMessages();

		if (!m_loopIsRunning)
		{
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(25));
	}

	if (m_threadIsRunning)
	{
		m_threadIsRunning = false;
	}
}

void MessageQueue::stopMessageLoop()
{
	if (!m_loopIsRunning)
	{
		LOG_WARNING("Loop is not running");
	}

	m_loopIsRunning = false;

	while (true)
	{
		if (!m_threadIsRunning)
		{
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(25));
	}
}

bool MessageQueue::loopIsRunning() const
{
	return m_loopIsRunning;
}

bool MessageQueue::hasMessagesQueued() const
{
	return !m_messageBuffer.access()->empty();
}

void MessageQueue::setSendMessagesAsTasks(bool sendMessagesAsTasks)
{
	m_sendMessagesAsTasks = sendMessagesAsTasks;
}

void MessageQueue::processMessages()
{
	while (true)
	{
		std::shared_ptr<MessageBase> message = aidkit::access([](auto &messageBuffer, auto &filters)
		{
			for (std::shared_ptr<MessageFilter> filter : filters)
			{
				if (messageBuffer.empty())
				{
					break;
				}
				filter->filter(&messageBuffer);
			}

			if (messageBuffer.empty())
			{
				return std::shared_ptr<MessageBase>(nullptr);
			}
			auto message = messageBuffer.front();
			messageBuffer.pop_front();

			return message;
		}, m_messageBuffer, m_filters);

		if (message == nullptr)
			break;

		processMessage(message, false);
	}
}

void MessageQueue::sendMessage(std::shared_ptr<MessageBase> message)
{
	auto listeners = m_listeners.access();

	// m_listenersLength is saved, so that new listeners registered within message handling don't
	// get the current message and the length can be reduced when a listener gets unregistered.
	m_listenersLength = listeners->size();

	// The currentListenerIndex holds the index of the current listener being handled, so it can be
	// changed when a listener gets removed while message handling.
	for (m_currentListenerIndex = 0; m_currentListenerIndex < m_listenersLength; m_currentListenerIndex++)
	{
		MessageListenerBase* listener = (*listeners)[m_currentListenerIndex];

		if (listener->getType() == message->getType()
			&& (message->getSchedulerId() == TabId::NONE || listener->getSchedulerId() == TabId::NONE
			|| listener->getSchedulerId() == message->getSchedulerId()))
		{
			// The listenersMutex gets unlocked so changes to listeners are possible while message handling.
			listeners.unlock();
			listener->handleMessageBase(message.get());
			listeners.lock();
		}
	}
}

void MessageQueue::sendMessageAsTask(std::shared_ptr<MessageBase> message, bool asNextTask) const
{
	std::shared_ptr<TaskGroup> taskGroup;
	if (message->isParallel())
	{
		taskGroup = std::make_shared<TaskGroupParallel>();
	}
	else
	{
		taskGroup = std::make_shared<TaskGroupSequence>();
	}

	aidkit::access([&message, &taskGroup](auto &listeners)
	{
		for (size_t i = 0; i < listeners.size(); i++)
		{
			MessageListenerBase* listener = listeners[i];

			if (listener->getType() == message->getType()
				&& (message->getSchedulerId() == TabId::NONE || listener->getSchedulerId() == TabId::NONE
				||  listener->getSchedulerId() == message->getSchedulerId()))
			{
				Id listenerId = listener->getId();
				taskGroup->addTask(std::make_shared<TaskLambda>([listenerId, message]()
				{
					MessageListenerBase *listener = MessageQueue::getInstance()->getListenerById(listenerId);
					if (listener)
					{
						listener->handleMessageBase(message.get());
					}
				}));
			}
		}
	}, m_listeners);

	TabId schedulerId = message->getSchedulerId();
	if (schedulerId == TabId::NONE)
	{
		schedulerId = TabIds::app();
	}

	if (asNextTask)
	{
		Task::dispatchNext(schedulerId, taskGroup);
	}
	else
	{
		Task::dispatch(schedulerId, taskGroup);
	}
}
