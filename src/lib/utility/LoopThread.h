#ifndef LOOP_THREAD_H
#define LOOP_THREAD_H

#include <atomic>
#include <thread>

class LoopThread
{
public:
	virtual ~LoopThread() = default;

	void startLoopThread();
	void stopLoopThread();

	bool isLoopRunning() const; // Only needed for unit tests!

protected:
	bool isLoopStopping() const;

private:
	enum class State
	{
		STOPPED, STARTING, RUNNING, STOPPING
	};

	void invokeThreadLoop();

	virtual void doThreadLoop() noexcept = 0;

	std::thread m_thread;
	std::atomic<State> m_state = State::STOPPED;
};

#endif
