#ifndef TASK_FILL_INDEXER_COMMAND_QUEUE_H
#define TASK_FILL_INDEXER_COMMAND_QUEUE_H

#include "MessageIndexingInterrupted.h"
#include "MessageListener.h"
#include "Task.h"

#include "InterprocessIndexerCommandManager.h"

#include <aidkit/thread_shared.hpp>

#include <queue>

class IndexerCommandProvider;

class TaskFillIndexerCommandsQueue
	: public Task
	, public MessageListener<MessageIndexingInterrupted>
{
public:
	TaskFillIndexerCommandsQueue(
		const std::string& appUUID,
		std::unique_ptr<IndexerCommandProvider> indexerCommandProvider,
		size_t maximumQueueSize);

protected:
	void doEnter(std::shared_ptr<Blackboard> blackboard) override;
	TaskState doUpdate(std::shared_ptr<Blackboard> blackboard) override;
	void doExit(std::shared_ptr<Blackboard> blackboard) override;
	void doReset(std::shared_ptr<Blackboard> blackboard) override;
	void terminate() override;

	void handleMessage(MessageIndexingInterrupted* message) override;

	bool fillCommandQueue();

private:
	const size_t m_maximumQueueSize;

	std::atomic<bool> m_interrupted = false;

	aidkit::thread_shared<std::unique_ptr<IndexerCommandProvider>> m_indexerCommandProvider;
	aidkit::thread_shared<InterprocessIndexerCommandManager> m_indexerCommandManager;
	aidkit::thread_shared<std::queue<FilePath>> m_filePathQueue;
};

#endif	  // TASK_FILL_INDEXER_COMMAND_QUEUE_H
