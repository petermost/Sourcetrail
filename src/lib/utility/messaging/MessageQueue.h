#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include "Id.h"

#include <aidkit/concurrent/thread_shared.hpp>

#include <atomic>
#include <deque>
#include <memory>
#include <thread>
#include <vector>

class MessageBase;
class MessageFilter;
class MessageListenerBase;

class MessageQueue
{
public:
	typedef std::deque<std::shared_ptr<MessageBase>> MessageBufferType;

	static std::shared_ptr<MessageQueue> getInstance();

	~MessageQueue() = default;

	void registerListener(MessageListenerBase* listener);
	void unregisterListener(MessageListenerBase* listener);

	MessageListenerBase* getListenerById(Id listenerId) const;

	void addMessageFilter(std::shared_ptr<MessageFilter> filter);

	void pushMessage(std::shared_ptr<MessageBase> message);
	void processMessage(std::shared_ptr<MessageBase> message, bool asNextTask);

	void startMessageLoopThread();
	void stopMessageLoopThread();

	bool isLoopRunning() const;
	bool hasMessagesQueued() const;

	bool setSendMessagesAsTasks(bool sendMessagesAsTasks);

private:
	MessageQueue() = default;

	MessageQueue(const MessageQueue&) = delete;
	MessageQueue &operator=(const MessageQueue&) = delete;

	void messageLoop();

	void sendMessage(std::shared_ptr<MessageBase> message);
	void sendMessageAsTask(std::shared_ptr<MessageBase> message, bool asNextTask) const;

	std::thread m_thread;
	std::atomic<bool> m_isLoopRunning = false;

	aidkit::concurrent::thread_shared<MessageBufferType> m_messageBuffer;
	aidkit::concurrent::thread_shared<std::vector<std::shared_ptr<MessageFilter>>> m_filters;

	aidkit::concurrent::thread_shared<std::size_t, std::recursive_mutex> m_currentListenerIndex = 0;
	aidkit::concurrent::thread_shared<std::vector<MessageListenerBase *>, std::recursive_mutex> m_listeners;
	aidkit::concurrent::thread_shared<std::size_t, std::recursive_mutex> m_currentListenersLength = 0;

	bool m_sendMessagesAsTasks = false;
};

#endif	  // MESSAGE_QUEUE_H
