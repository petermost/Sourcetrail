#ifndef TASK_BUILD_INDEX_H
#define TASK_BUILD_INDEX_H


#include "MessageIndexingInterrupted.h"
#include "MessageListener.h"
#include "Task.h"
#include "InterprocessIndexingStatusManager.h"
#include "InterprocessIntermediateStorageManager.h"

#include <atomic>
#include <thread>

class DialogView;
class StorageProvider;
class IndexerCommandList;

class TaskBuildIndex
	: public Task
	, public MessageListener<MessageIndexingInterrupted>
{
public:
	TaskBuildIndex(
		size_t processCount,
		std::shared_ptr<StorageProvider> storageProvider,
		std::shared_ptr<DialogView> dialogView,
		const std::string& appUUID,
		bool multiProcessIndexing);

protected:
	void doEnter(std::shared_ptr<Blackboard> blackboard) override;
	TaskState doUpdate(std::shared_ptr<Blackboard> blackboard) override;
	void doExit(std::shared_ptr<Blackboard> blackboard) override;
	void doReset(std::shared_ptr<Blackboard> blackboard) override;
	void terminate() override;

	void handleMessage(MessageIndexingInterrupted* message) override;
	
	void runIndexerProcess(ProcessId processId, const std::string& logFilePath);
	void runIndexerThread(ProcessId processId);
	bool fetchIntermediateStorages(std::shared_ptr<Blackboard> blackboard);
	void updateIndexingDialog(
		std::shared_ptr<Blackboard> blackboard, const std::vector<FilePath>& sourcePaths);

	static const std::string s_processName;

	std::shared_ptr<IndexerCommandList> m_indexerCommandList;
	std::shared_ptr<StorageProvider> m_storageProvider;
	std::shared_ptr<DialogView> m_dialogView;
	const std::string m_appUUID;
	bool m_multiProcessIndexing;

	InterprocessIndexingStatusManager m_interprocessIndexingStatusManager;
	bool m_indexerCommandQueueStopped = false;
	size_t m_processCount;
	bool m_interrupted = false;
	size_t m_indexingFileCount = 0;

	// store as plain pointers to avoid deallocation issues when closing app during indexing
	std::vector<std::thread*> m_processThreads;
	std::vector<std::shared_ptr<InterprocessIntermediateStorageManager>>
		m_interprocessIntermediateStorageManagers;

	std::atomic<size_t> m_runningThreadCount = 0;
};

#endif	  // TASK_PARSE_H
