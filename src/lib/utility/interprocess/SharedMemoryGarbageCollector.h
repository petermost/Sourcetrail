#ifndef SHARED_MEMORY_GARBAGE_COLLECTOR_H
#define SHARED_MEMORY_GARBAGE_COLLECTOR_H

#include <atomic>
#include <memory>
#include <mutex>
#include <set>
#include <string>

#include "SharedMemory.h"

#include <mutex>
#include <thread>

class SharedMemoryGarbageCollector
{
public:
	static SharedMemoryGarbageCollector* createInstance();
	static SharedMemoryGarbageCollector* getInstance();

	SharedMemoryGarbageCollector();
	~SharedMemoryGarbageCollector();

	void run(const std::string& uuid);
	void stop();

	void registerSharedMemory(const std::string& sharedMemoryName);
	void unregisterSharedMemory(const std::string& sharedMemoryName);

private:
	void update();

	static const std::string s_memoryName;
	static const std::string s_instancesKeyName;
	static const std::string s_timeStampsKeyName;

	static const size_t s_updateIntervalSeconds;
	static const size_t s_deleteThresholdSeconds;

	static std::shared_ptr<SharedMemoryGarbageCollector> s_instance;

	SharedMemory m_memory;
	std::atomic<bool> m_loopIsRunning;
	std::shared_ptr<std::thread> m_thread;

	std::string m_uuid;

	std::mutex m_sharedMemoryNamesMutex;
	std::set<std::string> m_sharedMemoryNames;
	std::set<std::string> m_removedSharedMemoryNames;
};

#endif	  // SHARED_MEMORY_GARBAGE_COLLECTOR_H
