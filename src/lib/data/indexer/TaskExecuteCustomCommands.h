#ifndef TASK_EXECUTE_CUSTOM_COMMANDS_H
#define TASK_EXECUTE_CUSTOM_COMMANDS_H

#include "ErrorCountInfo.h"
#include "FilePath.h"
#include "MessageIndexingInterrupted.h"
#include "MessageListener.h"
#include "Task.h"
#include "TimeStamp.h"

#include <aidkit/thread_shared.hpp>

#include <set>
#include <vector>

class DialogView;
class IndexerCommandCustom;
class IndexerCommandProvider;
class PersistentStorage;

class TaskExecuteCustomCommands
	: public Task
	, public MessageListener<MessageIndexingInterrupted>
{
public:
	static void runPythonPostProcessing(PersistentStorage& storage);

	TaskExecuteCustomCommands(
		std::unique_ptr<IndexerCommandProvider> indexerCommandProvider,
		std::shared_ptr<PersistentStorage> storage,
		std::shared_ptr<DialogView> dialogView,
		size_t indexerThreadCount,
		const FilePath& projectDirectory);

private:
	void doEnter(std::shared_ptr<Blackboard> blackboard) override;
	TaskState doUpdate(std::shared_ptr<Blackboard> blackboard) override;
	void doExit(std::shared_ptr<Blackboard> blackboard) override;
	void doReset(std::shared_ptr<Blackboard> blackboard) override;

	void handleMessage(MessageIndexingInterrupted* message) override;

	void executeParallelIndexerCommands(int threadId, std::shared_ptr<Blackboard> blackboard);
	void runIndexerCommand(
		std::shared_ptr<IndexerCommandCustom> indexerCommand,
		std::shared_ptr<Blackboard> blackboard,
		std::shared_ptr<PersistentStorage> storage);

	std::unique_ptr<IndexerCommandProvider> const m_indexerCommandProvider;
	std::shared_ptr<PersistentStorage> m_storage;
	std::shared_ptr<DialogView> const m_dialogView;
	const size_t m_indexerThreadCount;
	const FilePath m_projectDirectory;

	TimeStamp m_start;
	std::atomic<bool> m_interrupted = false;
	const size_t m_indexerCommandCount;
	std::vector<std::shared_ptr<IndexerCommandCustom>> m_serialCommands;
	aidkit::thread_shared<std::vector<std::shared_ptr<IndexerCommandCustom>>> m_parallelCommands;
	aidkit::thread_shared<ErrorCountInfo> m_errorCount;
	FilePath m_targetDatabaseFilePath;
	const bool m_hasPythonCommands = false; // TODO (petermost) Remove?
	aidkit::thread_shared<std::set<FilePath>> m_sourceDatabaseFilePaths;
};

#endif	  // TASK_EXECUTE_CUSTOM_COMMANDS_H
