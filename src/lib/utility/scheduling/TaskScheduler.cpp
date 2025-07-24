#include "TaskScheduler.h"

#include <chrono>
#include <thread>

#include "ScopedFunctor.h"
#include "logging.h"

TaskScheduler::TaskScheduler(TabId schedulerId)
	: m_schedulerId(schedulerId)
{
}

TaskScheduler::~TaskScheduler()
{
	stopSchedulerLoop();
}

void TaskScheduler::pushTask(std::shared_ptr<Task> task)
{
	m_taskRunners.access()->push_back(std::make_shared<TaskRunner>(task));
}

void TaskScheduler::pushNextTask(std::shared_ptr<Task> task)
{
	aidkit::access([&task](auto &taskRunners)
	{
		if (taskRunners.empty())
		{
			taskRunners.push_front(std::make_shared<TaskRunner>(task));
		}
		else
		{
			taskRunners.insert(taskRunners.begin() + 1, std::make_shared<TaskRunner>(task));
		}
	}, m_taskRunners);
}

void TaskScheduler::startSchedulerLoopThreaded()
{
	std::thread(&TaskScheduler::startSchedulerLoop, this).detach();

	m_threadIsRunning = true;
}

void TaskScheduler::startSchedulerLoop()
{
	if (m_loopIsRunning)
	{
		LOG_ERROR("Unable to start task scheduler. Loop is already running.");
		return;
	}

	m_loopIsRunning = true;

	while (true)
	{
		processTasks();

		if (!m_loopIsRunning)
		{
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(25));
	}

	if (m_threadIsRunning)
	{
		m_threadIsRunning = false;
	}
}

void TaskScheduler::stopSchedulerLoop()
{
	if (!m_loopIsRunning)
	{
		LOG_WARNING("Unable to stop task scheduler. Loop is not running.");
	}
	m_loopIsRunning = false;

	while (true)
	{
		if (!m_threadIsRunning)
		{
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(25));
	}
}

bool TaskScheduler::loopIsRunning() const
{
	return m_loopIsRunning;
}

bool TaskScheduler::hasTasksQueued() const
{
	return !m_taskRunners.access()->empty();
}

void TaskScheduler::terminateRunningTasks()
{
	m_terminateRunningTasks = true;
}

void TaskScheduler::processTasks()
{
	auto taskRunners = m_taskRunners.access();

	while (!taskRunners->empty())
	{
		std::shared_ptr<TaskRunner> runner = taskRunners->front();
		Task::TaskState state = Task::STATE_RUNNING;
		{
			taskRunners.unlock();

			[[maybe_unused]]
			ScopedFunctor functor([&taskRunners]()
			{
				taskRunners.lock();
			});

			while (true)
			{
				if (!m_loopIsRunning || m_terminateRunningTasks)
				{
					runner->terminate();
					break;
				}

				state = runner->update(m_schedulerId);
				if (state != Task::STATE_RUNNING)
				{
					break;
				}
			}
		}

		taskRunners->pop_front();

		if (state == Task::STATE_HOLD)
		{
			taskRunners->push_back(runner);
		}
	}
	m_terminateRunningTasks = false;
}
