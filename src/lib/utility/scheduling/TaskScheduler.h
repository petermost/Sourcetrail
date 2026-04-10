#ifndef TASK_SCHEDULER_H
#define TASK_SCHEDULER_H

#include "LoopThread.h"
#include "Task.h"
#include "TaskRunner.h"

#include <aidkit/concurrent/thread_shared.hpp>

#include <atomic>
#include <deque>
#include <memory>

class TaskScheduler final : public LoopThread
{
public:
	TaskScheduler(TabId schedulerId);
	~TaskScheduler() override;

	void pushTask(std::shared_ptr<Task> task);
	void pushNextTask(std::shared_ptr<Task> task);

	using LoopThread::isLoopRunning; // Only needed for unit tests!
	bool hasTasksQueued() const; // Only needed for unit tests!

	void terminateRunningTasks();

private:
	void doThreadLoop() noexcept override;
	void processTasks();

	const TabId m_schedulerId;

	std::atomic<bool> m_terminateRunningTasks = false;

	aidkit::concurrent::thread_shared<std::deque<std::shared_ptr<TaskRunner>>> m_taskRunners;
};

#endif	  // TASK_SCHEDULER_H
