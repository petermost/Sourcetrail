#include "LoopThread.h"

#include "logging.h"

void LoopThread::startLoopThread()
{
	State stoppedExpected = State::STOPPED;
	if (m_state.compare_exchange_strong(stoppedExpected, State::STARTING))
	{
		m_thread = std::thread(&LoopThread::invokeThreadLoop, this);
		m_state.wait(State::STARTING);
	}
	else
	{
		LOG_ERROR("Loop thread is already running");
	}
}

void LoopThread::stopLoopThread()
{
	// Signal the loop/thread to stop:

	State runningExpected = State::RUNNING;
	if (m_state.compare_exchange_strong(runningExpected, State::STOPPING))
	{
		m_thread.join();
		// m_state.wait(State::STOPPING);
	}
	else
	{
		LOG_ERROR("Loop thread is not running");
	}
}

void LoopThread::invokeThreadLoop()
{
	m_state = State::RUNNING;
	m_state.notify_one();

	doThreadLoop();

	m_state = State::STOPPED;
	m_state.notify_one();
}

bool LoopThread::isLoopRunning() const
{
	return m_state == State::RUNNING;
}

bool LoopThread::isLoopStopping() const
{
	return m_state == State::STOPPING;
}
