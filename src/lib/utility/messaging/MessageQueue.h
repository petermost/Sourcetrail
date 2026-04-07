#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include "Id.h"
#include "MessageListenerBase.h"

#include <aidkit/concurrent/thread_shared.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/tag.hpp>
#include <boost/multi_index/key.hpp>

#include <atomic>
#include <deque>
#include <memory>
#include <thread>
#include <vector>

class MessageBase;
class MessageFilter;

class MessageQueue
{
public:
	typedef std::deque<std::shared_ptr<MessageBase>> MessageBufferType;

	static std::shared_ptr<MessageQueue> getInstance();

	~MessageQueue() = default;

	void registerListener(MessageListenerBase* listener);
	void unregisterListener(MessageListenerBase* listener);

	void addMessageFilter(std::shared_ptr<MessageFilter> filter);

	void pushMessage(std::shared_ptr<MessageBase> message);
	void processMessage(std::shared_ptr<MessageBase> message, bool asNextTask);

	void startMessageLoopThread();
	void stopMessageLoopThread();

	bool isLoopRunning() const;
	bool hasMessagesQueued() const;

	bool setSendMessagesAsTasks(bool sendMessagesAsTasks);

private:
	// Declare a container which allows sequential and hashed access:

	struct ListenerSequenceTag {};
	struct ListenerHashTag {};

	using ListenerSequenceIndex = boost::multi_index::sequenced<boost::multi_index::tag<ListenerSequenceTag>>;
	using ListenerHashIndex = boost::multi_index::hashed_unique<boost::multi_index::tag<ListenerHashTag>, boost::multi_index::identity<MessageListenerBase *>>;

	using ListenerContainer = boost::multi_index::multi_index_container<
		MessageListenerBase *,
		boost::multi_index::indexed_by<
			ListenerSequenceIndex,
			ListenerHashIndex
		>
	>;

	MessageQueue() = default;

	MessageQueue(const MessageQueue&) = delete;
	MessageQueue &operator=(const MessageQueue&) = delete;

	void messageLoop();

	void sendMessage(std::shared_ptr<MessageBase> message) const;
	void sendMessageAsTask(std::shared_ptr<MessageBase> message, bool asNextTask) const;

	bool isListenerRegistered(const MessageListenerBase *listener) const;

	std::thread m_thread;
	std::atomic<bool> m_isLoopRunning = false;

	aidkit::concurrent::thread_shared<MessageBufferType> m_messageBuffer;
	aidkit::concurrent::thread_shared<std::vector<std::shared_ptr<MessageFilter>>> m_filters;

	aidkit::concurrent::thread_shared<ListenerContainer> m_listeners;

	bool m_sendMessagesAsTasks = false;
};

#endif	  // MESSAGE_QUEUE_H
