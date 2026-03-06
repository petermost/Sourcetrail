#include "Catch2.hpp"

#include "Message.h"
#include "MessageListener.h"
#include "MessageQueue.h"

#include <chrono>
#include <thread>

// IMPORTANT: Ensure that MessageQueue::m_sendMessagesAsTasks is set to false, otherwise the test will fail!

namespace
{
class TestMessage: public Message<TestMessage>
{
public:
	static const std::string getStaticType()
	{
		return "TestMessage";
	}
};

class Test2Message: public Message<Test2Message>
{
public:
	static const std::string getStaticType()
	{
		return "TestMessage2";
	}
};

class TestMessageListener: public MessageListener<TestMessage>
{
public:
	TestMessageListener() = default;

	int m_messageCount = 0;

private:
	void handleMessage(TestMessage*  /*message*/) override
	{
		m_messageCount++;
	}
};

class Test2MessageListener: public MessageListener<Test2Message>
{
public:
	Test2MessageListener() = default;

	int m_messageCount = 0;

private:
	void handleMessage(Test2Message*  /*message*/) override
	{
		m_messageCount++;
		TestMessage().dispatch();
	}
};

class Test3MessageListener: public MessageListener<Test2Message>
{
public:
	std::shared_ptr<TestMessageListener> m_listener;

private:
	void handleMessage(Test2Message*  /*message*/) override
	{
		m_listener = std::make_shared<TestMessageListener>();
	}
};

class Test4MessageListener
	: public MessageListener<TestMessage>
	, public MessageListener<Test2Message>
{
public:
	std::shared_ptr<TestMessageListener> m_listener;

private:
	void handleMessage(TestMessage*  /*message*/) override
	{
		if (!m_listener)
		{
			m_listener = std::make_shared<TestMessageListener>();
		}
	}

	void handleMessage(Test2Message*  /*message*/) override
	{
		m_listener.reset();
	}
};

class Test5MessageListener: public MessageListener<TestMessage>
{
public:
	std::vector<std::shared_ptr<TestMessageListener>> m_listeners;

private:
	void handleMessage(TestMessage*  /*message*/) override
	{
		if (m_listeners.empty())
		{
			for (size_t i = 0; i < 5; i++)
			{
				m_listeners.push_back(std::make_shared<TestMessageListener>());
			}
		}
	}
};

void waitForThread()
{
	static const int THREAD_WAIT_TIME_MS = 20;

	while (MessageQueue::getInstance()->hasMessagesQueued())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_WAIT_TIME_MS));
	}
}

}	 // namespace

TEST_CASE("message queue loop starts and stops")
{
	REQUIRE(!MessageQueue::getInstance()->isLoopRunning());

	MessageQueue::getInstance()->startMessageLoopThread();

	waitForThread();

	REQUIRE(MessageQueue::getInstance()->isLoopRunning());

	MessageQueue::getInstance()->stopMessageLoopThread();

	waitForThread();

	REQUIRE(!MessageQueue::getInstance()->isLoopRunning());
}

TEST_CASE("message queue registered listener receives messages")
{
	MessageQueue::getInstance()->startMessageLoopThread();

	TestMessageListener listener;
	Test2MessageListener listener2;

	TestMessage().dispatch();
	TestMessage().dispatch();
	TestMessage().dispatch();

	waitForThread();

	MessageQueue::getInstance()->stopMessageLoopThread();

	REQUIRE(3 == listener.m_messageCount);
	REQUIRE(0 == listener2.m_messageCount);
}

TEST_CASE("message queue dispatching within message handling") // FAIL
{
	MessageQueue::getInstance()->startMessageLoopThread();

	TestMessageListener listener;
	Test2MessageListener listener2;

	Test2Message().dispatch();

	waitForThread();

	MessageQueue::getInstance()->stopMessageLoopThread();

	REQUIRE(1 == listener.m_messageCount);
	REQUIRE(1 == listener2.m_messageCount);
}

TEST_CASE("message queue listener registration within message handling")
{
	MessageQueue::getInstance()->startMessageLoopThread();

	Test3MessageListener listener;

	Test2Message().dispatch();
	TestMessage().dispatch();

	waitForThread();

	MessageQueue::getInstance()->stopMessageLoopThread();

	REQUIRE(listener.m_listener);
	if (listener.m_listener)
	{
		REQUIRE(1 == listener.m_listener->m_messageCount);
	}
}

TEST_CASE("message queue listener unregistration within message handling")
{
	MessageQueue::getInstance()->startMessageLoopThread();

	Test4MessageListener listener;

	TestMessage().dispatch();

	Test2Message().dispatch();

	TestMessage().dispatch();
	TestMessage().dispatch();
	TestMessage().dispatch();

	waitForThread();

	MessageQueue::getInstance()->stopMessageLoopThread();

	REQUIRE(listener.m_listener);
	if (listener.m_listener)
	{
		REQUIRE(2 == listener.m_listener->m_messageCount);
	}
}

TEST_CASE("message queue listener registration to front and back within message handling")
{
	MessageQueue::getInstance()->startMessageLoopThread();

	Test5MessageListener listener;

	TestMessage().dispatch();
	TestMessage().dispatch();
	TestMessage().dispatch();

	waitForThread();

	MessageQueue::getInstance()->stopMessageLoopThread();

	REQUIRE(5 == listener.m_listeners.size());
	REQUIRE(2 == listener.m_listeners[0]->m_messageCount);
	REQUIRE(2 == listener.m_listeners[1]->m_messageCount);
	REQUIRE(2 == listener.m_listeners[2]->m_messageCount);
	REQUIRE(2 == listener.m_listeners[3]->m_messageCount);
	REQUIRE(2 == listener.m_listeners[4]->m_messageCount);
}
