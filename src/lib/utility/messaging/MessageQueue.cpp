#include "MessageQueue.h"

#include "Message.h"
#include "MessageFilter.h"
#include "MessageListenerBase.h"
#include "TabIds.h"
#include "TaskGroupParallel.h"
#include "TaskGroupSequence.h"
#include "TaskLambda.h"
#include "logging.h"

#include <boost/preprocessor/stringize.hpp>
#include <boost/container/small_vector.hpp>

#include <chrono>
#include <thread>

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

// We can't use std::shared_ptr<> for the listener, because:
// 1) MessageListenerBase ctor calls registerListener() which would call shared_from_this() which will fail when called from a ctor!
// 2) Some listeners are derived from QObject/QWidget and therefore managed by Qt itself!
//    Example: QtSearchBarButton -> QtSelfRefreshIconButton -> QPushButton | MessageListener<MessageRefreshUI>

void MessageQueue::registerListener(MessageListenerBase* listener)
{
	aidkit::concurrent::access([=](ListenerContainer &listeners)
	{
		listeners.push_back(listener);
	}, m_listeners);
}

void MessageQueue::unregisterListener(MessageListenerBase* listener)
{
	aidkit::concurrent::access([=](ListenerContainer &listeners)
	{
		auto &listenerHashedIndex = listeners.get<ListenerHashTag>();

		if (listenerHashedIndex.erase(listener) == 0)
		{
			LOG_ERROR("Listener was not found");
		}
	}, m_listeners);
}

bool MessageQueue::isListenerRegistered(const MessageListenerBase *listener) const
{
	return aidkit::concurrent::access([=](const ListenerContainer &listeners)
	{
		const auto &listenerHashedIndex = listeners.get<ListenerHashTag>();

		return listenerHashedIndex.find(const_cast<MessageListenerBase *>(listener)) != listenerHashedIndex.end();
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


void MessageQueue::sendMessage(std::shared_ptr<MessageBase> message) const
{
	using SmallListenerVector = boost::container::small_vector<MessageListenerBase *, 10>;

	// Appearantly calling handleMessageBase() while holding the lock, will block QtThreadedFunctor/QtThreadedLambdaFunctor listeners,
	// so retrieve/store the matching listeners in a separate container:

	SmallListenerVector matchingListeners = aidkit::concurrent::access([=](const ListenerContainer &listeners)
	{
		SmallListenerVector matches;

		for (MessageListenerBase *listener : listeners)
		{
			if (isListenerMatch(message, listener))
			{
				matches.push_back(listener);
			}
		}
		return matches;
	}, m_listeners);

	// Now call handleMessageBase() after the lock was released.
	// Note: In theory the listeners could become invalid right after the mutex is released (https://en.wikipedia.org/wiki/Time-of-check_to_time-of-use)
	// But the original code used a similar approach, so it seems to be safe.
	// https://github.com/petermost/Sourcetrail/blob/3a75ef0df450466945e3a78f57ce0da5f46645cd/src/lib/utility/messaging/MessageQueue.cpp#L261:

	for (MessageListenerBase *listener : matchingListeners)
	{
		if (isListenerRegistered(listener))
		{
			listener->handleMessageBase(message.get());
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

	aidkit::concurrent::access([=, this](const ListenerContainer &listeners)
	{
		for (MessageListenerBase *listener : listeners)
		{
			if (isListenerMatch(message, listener))
			{
				taskGroup->addTask(std::make_shared<TaskLambda>([=, this]()
				{
					if (isListenerRegistered(listener))
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
