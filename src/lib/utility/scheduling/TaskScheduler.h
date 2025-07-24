#ifndef TASK_SCHEDULER_H
#define TASK_SCHEDULER_H

#include "Task.h"
#include "TaskRunner.h"

#include <aidkit/thread_shared.hpp>

#include <deque>
#include <memory>

class TaskScheduler
{
public:
	TaskScheduler(TabId schedulerId);
	~TaskScheduler();

	void pushTask(std::shared_ptr<Task> task);
	void pushNextTask(std::shared_ptr<Task> task);

	void startSchedulerLoopThreaded();
	void startSchedulerLoop();
	void stopSchedulerLoop();

	bool loopIsRunning() const;
	bool hasTasksQueued() const;

	void terminateRunningTasks();

private:
	void processTasks();

	const TabId m_schedulerId;

	std::atomic<bool> m_loopIsRunning = false;
	std::atomic<bool> m_threadIsRunning = false;
	std::atomic<bool> m_terminateRunningTasks = false;

	aidkit::thread_shared<std::deque<std::shared_ptr<TaskRunner>>> m_taskRunners;
};

#endif	  // TASK_SCHEDULER_H
