#include "TaskManager.h"

#include "TaskScheduler.h"

aidkit::thread_shared<std::map<TabId, std::shared_ptr<TaskScheduler>>> TaskManager::s_schedulers;

std::shared_ptr<TaskScheduler> TaskManager::createScheduler(TabId schedulerId)
{
	return getScheduler(schedulerId);
}

void TaskManager::destroyScheduler(TabId schedulerId)
{
	aidkit::access([schedulerId](auto &schedulers)
	{
		auto it = schedulers.find(schedulerId);
		if (it != schedulers.end())
		{
			schedulers.erase(it);
		}
	}, s_schedulers);
}

std::shared_ptr<TaskScheduler> TaskManager::getScheduler(TabId schedulerId)
{
	return aidkit::access([schedulerId](auto &schedulers)
	{
		auto it = schedulers.find(schedulerId);
		if (it != schedulers.end())
		{
			return it->second;
		}

		std::shared_ptr<TaskScheduler> scheduler = std::make_shared<TaskScheduler>(schedulerId);
		schedulers.emplace(schedulerId, scheduler);

		return scheduler;
	}, s_schedulers);
}
