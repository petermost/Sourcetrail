#include "TaskScheduler.h"

#include <chrono>
#include <thread>

TaskScheduler::TaskScheduler(TabId schedulerId)
	: m_schedulerId(schedulerId)
{
}

TaskScheduler::~TaskScheduler()
{
	stopLoopThread();
}

void TaskScheduler::pushTask(std::shared_ptr<Task> task)
{
	m_taskRunners.access()->push_back(std::make_shared<TaskRunner>(task));
}

void TaskScheduler::pushNextTask(std::shared_ptr<Task> task)
{
	aidkit::concurrent::access([=](std::deque<std::shared_ptr<TaskRunner>> &taskRunners)
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

bool TaskScheduler::hasTasksQueued() const
{
	return !m_taskRunners.access()->empty();
}

void TaskScheduler::terminateRunningTasks()
{
	m_terminateRunningTasks = true;
}

void TaskScheduler::doThreadLoop() noexcept
{
	for (;;)
	{
		processTasks();

		if (isLoopStopping())
		{
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(25));
	}
}

void TaskScheduler::processTasks()
{
	auto tryFrontFunctor = [](std::deque<std::shared_ptr<TaskRunner>> &taskRunners) -> std::shared_ptr<TaskRunner>
	{
		if (!taskRunners.empty())
			return taskRunners.front();
		else
			return nullptr;
	};

	std::shared_ptr<TaskRunner> runner;
	while ((runner = aidkit::concurrent::access(tryFrontFunctor, m_taskRunners)))
	{
		Task::TaskState state = Task::STATE_RUNNING;

		for (;;)
		{
			if (isLoopStopping() || m_terminateRunningTasks)
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

		aidkit::concurrent::access([=](std::deque<std::shared_ptr<TaskRunner>> &taskRunners)
		{
			taskRunners.pop_front();

			if (state == Task::STATE_HOLD)
			{
				taskRunners.push_back(runner);
			}
		}, m_taskRunners);
	}

	m_terminateRunningTasks = false;
}
