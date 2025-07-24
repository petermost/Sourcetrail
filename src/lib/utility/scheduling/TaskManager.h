#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include "TabIds.h"
#include "TaskScheduler.h"

#include <aidkit/thread_shared.hpp>

#include <map>
#include <memory>

class TaskManager
{
public:
	static std::shared_ptr<TaskScheduler> createScheduler(TabId schedulerId);
	static void destroyScheduler(TabId schedulerId);
	
	static std::shared_ptr<TaskScheduler> getScheduler(TabId schedulerId);

private:
	static aidkit::thread_shared<std::map<TabId, std::shared_ptr<TaskScheduler>>> s_schedulers;
};

#endif	  // TASK_MANAGER_H
