#include "MessageQueue.h"

#include "Message.h"
#include "MessageFilter.h"
#include "MessageListenerBase.h"
#include "TabIds.h"
#include "TaskGroupParallel.h"
#include "TaskGroupSequence.h"
#include "TaskLambda.h"
#include "logging.h"

#include <chrono>
#include <thread>

#include <boost/preprocessor/stringize.hpp>

using namespace std;

namespace
{

class MessageStopLoop: public Message<MessageStopLoop>
{
public:
	static const std::string getStaticType()
	{
		return BOOST_PP_STRINGIZE(MessageStopLoop);
	}
};

const std::shared_ptr STOP_LOOP_MESSAGE(std::make_shared<MessageStopLoop>());

}

std::shared_ptr<MessageQueue> MessageQueue::getInstance()
{
	static std::shared_ptr<MessageQueue> s_instance = std::shared_ptr<MessageQueue>(new MessageQueue());

	return s_instance;
}

void MessageQueue::registerListener(MessageListenerBase* listener)
{
	m_listeners.access()->push_back(listener);
}

void MessageQueue::unregisterListener(MessageListenerBase* listener)
{
	aidkit::concurrent::access([=](size_t &currentListenerIndex, std::vector<MessageListenerBase *> &listeners, size_t &currentListenersLength)
	{
		for (size_t listenerIndex = 0; listenerIndex < listeners.size(); ++listenerIndex)
		{
			if (listeners[listenerIndex] == listener)
			{
				listeners.erase(listeners.begin() + listenerIndex);

				// Adjust loop control variables for the removed listener:

				if (listenerIndex <= currentListenerIndex)
					--currentListenerIndex;

				if (listenerIndex < currentListenersLength)
					--currentListenersLength;

				return;
			}
		}
		LOG_ERROR("Listener was not found");
	}, m_currentListenerIndex, m_listeners, m_currentListenersLength);
}

MessageListenerBase* MessageQueue::getListenerById(Id listenerId) const
{
	return aidkit::concurrent::access([=](const std::vector<MessageListenerBase *> &listeners)
	{
		auto it = std::find_if(listeners.begin(), listeners.end(), [=](MessageListenerBase *listener)
		{
			return listener->getId() == listenerId;
		});
		return (it != listeners.end()) ? *it : nullptr;
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

void MessageQueue::startMessageLoopThread()
{
	if (!m_isLoopRunning)
	{
		m_thread = std::thread(&MessageQueue::messageLoop, this);

		m_isLoopRunning.wait(false);
	}
	else
	{
		LOG_ERROR("Loop is already running");
	}
}

void MessageQueue::stopMessageLoopThread()
{
	// Signal the loop/thread to stop:

	if (m_isLoopRunning)
	{
		pushMessage(STOP_LOOP_MESSAGE);

		m_thread.join();
	}
	else
	{
		LOG_ERROR("Loop is not running");
	}
}

bool MessageQueue::isLoopRunning() const
{
	return m_isLoopRunning;
}

bool MessageQueue::hasMessagesQueued() const
{
	return !m_messageBuffer.access()->empty();
}

bool MessageQueue::setSendMessagesAsTasks(bool sendMessagesAsTasks)
{
	bool previousValue = m_sendMessagesAsTasks;
	m_sendMessagesAsTasks = sendMessagesAsTasks;

	return previousValue;
}

void MessageQueue::messageLoop()
{
	m_isLoopRunning = true;
	m_isLoopRunning.notify_one();

	std::shared_ptr<MessageBase> message;

	while (message != STOP_LOOP_MESSAGE)
	{
		message = aidkit::concurrent::access([](MessageBufferType &messageBuffer, std::vector<std::shared_ptr<MessageFilter>> &filters)
		{
			for (std::shared_ptr<MessageFilter> filter : filters)
			{
				if (!messageBuffer.empty())
					filter->filter(&messageBuffer);
				else
					break;
			}

			if (!messageBuffer.empty())
			{
				std::shared_ptr<MessageBase> message(std::move(messageBuffer.front()));
				messageBuffer.pop_front();

				return message;
			}
			else
				return std::shared_ptr<MessageBase>(nullptr);
		}, m_messageBuffer, m_filters);

		if (message != nullptr)
			processMessage(message, false);
		else
			std::this_thread::sleep_for(std::chrono::milliseconds(25));
	}
	m_isLoopRunning = false;
	m_isLoopRunning.notify_one();
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

static inline bool isListenerMatch(const std::shared_ptr<MessageBase> &message, const MessageListenerBase *listener)
{
	return listener->getType() == message->getType()
		&& (message->getSchedulerId() == TabId::NONE || listener->getSchedulerId() == TabId::NONE
		|| listener->getSchedulerId() == message->getSchedulerId());
}

void MessageQueue::sendMessage(std::shared_ptr<MessageBase> message)
{
	aidkit::concurrent::access([=](size_t &currentListenerIndex, const std::vector<MessageListenerBase *> &listeners, size_t &currentListenersLength)
	{
		// Remember the length, so appended listeners are not getting called for the current message:
		// Note: It's unclear why that is important, but the unit tests check for this behaviour.

		currentListenersLength = listeners.size();

		for (currentListenerIndex = 0; currentListenerIndex < currentListenersLength; ++currentListenerIndex)
		{
			MessageListenerBase *listener = listeners[currentListenerIndex];

			if (isListenerMatch(message, listener))
			{
				listener->handleMessageBase(message.get());
			}
		}
	}, m_currentListenerIndex, m_listeners, m_currentListenersLength);
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

	aidkit::concurrent::access([=, this](const std::vector<MessageListenerBase *> &listeners)
	{
		for (size_t listenerIndex = 0; listenerIndex < listeners.size(); ++listenerIndex)
		{
			MessageListenerBase *listener = listeners[listenerIndex];

			if (isListenerMatch(message, listener))
			{
				Id listenerId = listener->getId();
				taskGroup->addTask(std::make_shared<TaskLambda>([=, this]()
				{
					MessageListenerBase* listener = getListenerById(listenerId);
					if (listener != nullptr)
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
