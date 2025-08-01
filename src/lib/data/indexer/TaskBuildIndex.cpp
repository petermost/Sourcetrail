#include "TaskBuildIndex.h"

#include "AppPath.h"
#include "Blackboard.h"
#include "DialogView.h"
#include "FileLogger.h"
#include "InterprocessIndexer.h"
#include "MessageIndexingStatus.h"
#include "ParserClientImpl.h"
#include "StorageProvider.h"
#include "TimeStamp.h"
#include "UserPaths.h"
#include "utilityApp.h"

using namespace utility;

TaskBuildIndex::TaskBuildIndex(
	size_t processCount,
	std::shared_ptr<StorageProvider> storageProvider,
	std::shared_ptr<DialogView> dialogView,
	const std::string& appUUID,
	bool multiProcessIndexing)
	: m_storageProvider(storageProvider)
	, m_dialogView(dialogView)
	, m_appUUID(appUUID)
	, m_multiProcessIndexing(multiProcessIndexing)
	, m_interprocessIndexingStatusManager(appUUID, ProcessId::NONE, true)
	,
	 m_processCount(processCount)

{
}

void TaskBuildIndex::doEnter(std::shared_ptr<Blackboard> blackboard)
{
	m_interprocessIndexingStatusManager.setIndexingInterrupted(false);

	m_indexingFileCount = 0;
	updateIndexingDialog(blackboard, std::vector<FilePath>());

	std::string logFilePath;
	Logger* logger = LogManager::getInstance()->getLoggerByType("FileLogger");
	if (logger != nullptr)
	{
		logFilePath = dynamic_cast<FileLogger*>(logger)->getLogFilePath().str();
	}

	// start indexer processes
	for (size_t i = 0; i < m_processCount; i++)
	{
		m_runningThreadCount++;

		const ProcessId processId = static_cast<ProcessId>(i + 1);	// 0 remains reserved for the main process

		m_interprocessIntermediateStorageManagers.push_back(std::make_shared<InterprocessIntermediateStorageManager>(m_appUUID, processId, true));

		if (m_multiProcessIndexing)
		{
			m_processThreads.push_back(new std::thread(&TaskBuildIndex::runIndexerProcess, this, processId, logFilePath));
		}
		else
		{
			m_processThreads.push_back(new std::thread(&TaskBuildIndex::runIndexerThread, this, processId));
		}
	}

	blackboard->set<bool>("indexer_threads_started", true);
}

Task::TaskState TaskBuildIndex::doUpdate(std::shared_ptr<Blackboard> blackboard)
{
	size_t runningThreadCount = m_runningThreadCount;

	blackboard->get<bool>("indexer_command_queue_stopped", m_indexerCommandQueueStopped);

	const std::vector<FilePath> indexingFiles = m_interprocessIndexingStatusManager.getCurrentlyIndexedSourceFilePaths();
	if (!indexingFiles.empty())
	{
		updateIndexingDialog(blackboard, indexingFiles);
	}

	if (m_indexerCommandQueueStopped && runningThreadCount == 0)
	{
		LOG_INFO_STREAM(<< "command queue stopped and no running threads. done.");
		return STATE_SUCCESS;
	}
	else if (m_interrupted)
	{
		LOG_INFO_STREAM(<< "interrupted indexing.");
		blackboard->set("interrupted_indexing", true);
		return STATE_SUCCESS;
	}

	if (fetchIntermediateStorages(blackboard))
	{
		updateIndexingDialog(blackboard, std::vector<FilePath>());
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(50));

	return STATE_RUNNING;
}

void TaskBuildIndex::doExit(std::shared_ptr<Blackboard> blackboard)
{
	for (auto *processThread: m_processThreads)
	{
		processThread->join();
		delete processThread;
	}
	m_processThreads.clear();

	if (!m_interrupted)
	{
		while (fetchIntermediateStorages(blackboard))
			;
	}

	std::vector<FilePath> crashedFiles =
		m_interprocessIndexingStatusManager.getCrashedSourceFilePaths();
	if (!crashedFiles.empty())
	{
		std::shared_ptr<IntermediateStorage> storage = std::make_shared<IntermediateStorage>();
		std::shared_ptr<ParserClientImpl> parserClient = std::make_shared<ParserClientImpl>(storage);

		for (const FilePath& path: crashedFiles)
		{
			Id fileId = parserClient->recordFile(path.getCanonical(), false);
			parserClient->recordError(
				"The translation unit threw an exception during indexing. Please check if the "
				"source file "
				"conforms to the specified language standard and all necessary options are defined "
				"within your project "
				"setup.",
				true,
				true,
				path,
				ParseLocation(fileId, 1, 1));
			LOG_INFO("crashed translation unit: " + path.str());
		}
		m_storageProvider->insert(storage);
	}

	blackboard->set<bool>("indexer_threads_stopped", true);
}

void TaskBuildIndex::doReset(std::shared_ptr<Blackboard>  /*blackboard*/) {}

void TaskBuildIndex::terminate()
{
	m_interrupted = true;
	utility::killRunningProcesses();
}

void TaskBuildIndex::handleMessage(MessageIndexingInterrupted*  /*message*/)
{
	LOG_INFO("sending indexer interrupt command.");

	m_interprocessIndexingStatusManager.setIndexingInterrupted(true);
	m_interrupted = true;

	m_dialogView->showUnknownProgressDialog(
		"Interrupting Indexing", "Waiting for indexer\nthreads to finish");
}

void TaskBuildIndex::runIndexerProcess(ProcessId processId, const std::string& logFilePath)
{
	const FilePath indexerProcessPath = AppPath::getCxxIndexerFilePath();
	if (!indexerProcessPath.exists())
	{
		m_interrupted = true;
		LOG_ERROR(
			"Cannot start indexer process because executable is missing at \"" +
			indexerProcessPath.str() + "\"");
		return;
	}

	std::vector<std::string> commandArguments;
	commandArguments.push_back(to_string(processId));
	commandArguments.push_back(m_appUUID);
	commandArguments.push_back(AppPath::getSharedDataDirectoryPath().getAbsolute().str());
	commandArguments.push_back(UserPaths::getUserDataDirectoryPath().getAbsolute().str());

	if (!logFilePath.empty())
	{
		commandArguments.push_back(logFilePath);
	}

	int result = 1;
	while ((!m_indexerCommandQueueStopped || result != 0) && !m_interrupted)
	{
		result = utility::executeProcess(
					 indexerProcessPath.str(), commandArguments, FilePath(), false, INFINITE_TIMEOUT)
					 .exitCode;

		LOG_INFO_STREAM(<< "Indexer process " << processId << " returned with " + std::to_string(result));
	}

	m_runningThreadCount--;
}

void TaskBuildIndex::runIndexerThread(ProcessId processId)
{
	do
	{
		InterprocessIndexer indexer(m_appUUID, processId);
		indexer.work();	   // this will only return if there are no indexer commands left in the queue
		if (!m_interrupted)
		{
			// sleeping if interrupted may result in a crash due to objects that are already
			// destroyed after waking up again
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
	} while (!m_indexerCommandQueueStopped && !m_interrupted);

	m_runningThreadCount--;
}

bool TaskBuildIndex::fetchIntermediateStorages(std::shared_ptr<Blackboard> blackboard)
{
	int poppedStorageCount = 0;

	int providerStorageCount = m_storageProvider->getStorageCount();
	if (providerStorageCount > 10)
	{
		LOG_INFO_STREAM(<< "waiting, too many storages queued: " << providerStorageCount);

		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		return true;
	}

	TimeStamp t = TimeStamp::now();
	do
	{
		ProcessId finishedProcessId = m_interprocessIndexingStatusManager.getNextFinishedProcessId();
		if (finishedProcessId == ProcessId::NONE 
			|| static_cast<size_t>(finishedProcessId) > m_interprocessIntermediateStorageManagers.size())
		{
			break;
		}

		std::shared_ptr<InterprocessIntermediateStorageManager> storageManager =
			m_interprocessIntermediateStorageManagers[static_cast<size_t>(finishedProcessId) - 1];

		const size_t storageCount = storageManager->getIntermediateStorageCount();
		if (storageCount == 0)
		{
			break;
		}

		LOG_INFO_STREAM(<< storageManager->getProcessId() << " - storage count: " << storageCount);
		m_storageProvider->insert(storageManager->popIntermediateStorage());
		poppedStorageCount++;
	} while (TimeStamp::now().deltaMS(t) < 500); // don't process all storages at once to allow for status updates in-between

	if (poppedStorageCount > 0)
	{
		blackboard->update<int>("indexed_source_file_count", [=](int count) { return count + poppedStorageCount; });
		return true;
	}

	return false;
}

void TaskBuildIndex::updateIndexingDialog(std::shared_ptr<Blackboard> blackboard, const std::vector<FilePath>& sourcePaths)
{
	// TODO: factor in unindexed files...
	int sourceFileCount = 0;
	int indexedSourceFileCount = 0;
	blackboard->get("source_file_count", sourceFileCount);
	blackboard->get("indexed_source_file_count", indexedSourceFileCount);

	m_indexingFileCount += sourcePaths.size();

	m_dialogView->updateIndexingDialog(m_indexingFileCount, indexedSourceFileCount, sourceFileCount, sourcePaths);

	int progress = 0;
	if (sourceFileCount != 0)
	{
		progress = indexedSourceFileCount * 100 / sourceFileCount;
	}
	MessageIndexingStatus(true, progress).dispatch();
}
