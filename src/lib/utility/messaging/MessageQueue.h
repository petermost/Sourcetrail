#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include "LoopThread.h"
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

#include <deque>
#include <memory>
#include <vector>

class MessageBase;
class MessageFilter;

class MessageQueue final : public LoopThread
{
public:
	typedef std::deque<std::shared_ptr<MessageBase>> MessageBufferType;

	static std::shared_ptr<MessageQueue> getInstance();

	void registerListener(MessageListenerBase* listener);
	void unregisterListener(MessageListenerBase* listener);

	void addMessageFilter(std::shared_ptr<MessageFilter> filter);

	void pushMessage(std::shared_ptr<MessageBase> message);
	void processMessage(std::shared_ptr<MessageBase> message, bool asNextTask);

	using LoopThread::isLoopRunning; // Only needed for unit tests!
	bool hasMessagesQueued() const; // Only needed for unit tests!

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

	void doThreadLoop() noexcept override;

	void sendMessage(std::shared_ptr<MessageBase> message) const;
	void sendMessageAsTask(std::shared_ptr<MessageBase> message, bool asNextTask) const;

	bool isListenerRegistered(const MessageListenerBase *listener) const;

	aidkit::concurrent::thread_shared<MessageBufferType> m_messageBuffer;
	aidkit::concurrent::thread_shared<std::vector<std::shared_ptr<MessageFilter>>> m_filters;

	aidkit::concurrent::thread_shared<ListenerContainer> m_listeners;

	bool m_sendMessagesAsTasks = false;
};

#endif	  // MESSAGE_QUEUE_H
