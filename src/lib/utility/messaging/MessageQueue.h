#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include "Id.h"

#include <atomic>
#include <deque>
#include <memory>
#include <vector>

#include <aidkit/thread_shared.hpp>

class MessageBase;
class MessageFilter;
class MessageListenerBase;

class MessageQueue
{
public:
	typedef std::deque<std::shared_ptr<MessageBase>> MessageBufferType;

	static std::shared_ptr<MessageQueue> getInstance();

	~MessageQueue();

	void registerListener(MessageListenerBase* listener);
	void unregisterListener(MessageListenerBase* listener);

	MessageListenerBase* getListenerById(Id listenerId) const;

	void addMessageFilter(std::shared_ptr<MessageFilter> filter);

	void pushMessage(std::shared_ptr<MessageBase> message);
	void processMessage(std::shared_ptr<MessageBase> message, bool asNextTask);

	void startMessageLoopThreaded();
	void startMessageLoop();
	void stopMessageLoop();

	bool loopIsRunning() const;
	bool hasMessagesQueued() const;

	void setSendMessagesAsTasks(bool sendMessagesAsTasks);

private:
	static std::shared_ptr<MessageQueue> s_instance;

	MessageQueue() = default;
	MessageQueue(const MessageQueue&) = delete;
	MessageQueue &operator=(const MessageQueue&) = delete;

	void processMessages();
	void sendMessage(std::shared_ptr<MessageBase> message);
	void sendMessageAsTask(std::shared_ptr<MessageBase> message, bool asNextTask) const;

	aidkit::thread_shared<MessageBufferType> m_messageBuffer;
	aidkit::thread_shared<std::vector<std::shared_ptr<MessageFilter>>> m_filters;

	aidkit::thread_shared<std::vector<MessageListenerBase*>> m_listeners;
	std::atomic<size_t> m_currentListenerIndex = 0;
	std::atomic<size_t> m_listenersLength = 0;

	std::atomic<bool> m_loopIsRunning = false;
	std::atomic<bool> m_threadIsRunning = false;

	bool m_sendMessagesAsTasks = false;
};

#endif	  // MESSAGE_QUEUE_H
