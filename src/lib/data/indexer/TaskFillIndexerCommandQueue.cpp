#include "TaskFillIndexerCommandQueue.h"

#include "Blackboard.h"
#include "IndexerCommandProvider.h"
#include "logging.h"
#include "utilityFile.h"

TaskFillIndexerCommandsQueue::TaskFillIndexerCommandsQueue(
	const std::string& appUUID,
	std::unique_ptr<IndexerCommandProvider> indexerCommandProvider,
	size_t maximumQueueSize)
	: m_maximumQueueSize(maximumQueueSize)
	, m_indexerCommandProvider(std::move(indexerCommandProvider))
	, m_indexerCommandManager(appUUID, ProcessId::NONE, true)
{
}

void TaskFillIndexerCommandsQueue::doEnter(std::shared_ptr<Blackboard> blackboard)
{
	aidkit::access([](auto &indexerCommandProvider, auto &filePathQueue)
	{
		for (const FilePath& filePath : utility::partitionFilePathsBySize(indexerCommandProvider->getAllSourceFilePaths(), 2))
		{
			filePathQueue.emplace(filePath);
		}
	}, m_indexerCommandProvider, m_filePathQueue);

	fillCommandQueue();

	blackboard->set<bool>("indexer_command_queue_started", true);
}

Task::TaskState TaskFillIndexerCommandsQueue::doUpdate(std::shared_ptr<Blackboard>  /*blackboard*/)
{
	if (m_interrupted)
	{
		return STATE_FAILURE;
	}

	if (!fillCommandQueue())
	{
		if ((*m_indexerCommandProvider.access())->empty())
		{
			return STATE_SUCCESS;
		}
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(200));

	return STATE_RUNNING;
}

void TaskFillIndexerCommandsQueue::doExit(std::shared_ptr<Blackboard> blackboard)
{
	blackboard->set<bool>("indexer_command_queue_stopped", true);
}

void TaskFillIndexerCommandsQueue::doReset(std::shared_ptr<Blackboard>  /*blackboard*/)
{
	m_interrupted = false;
}

void TaskFillIndexerCommandsQueue::terminate()
{
	m_interrupted = true;
}

void TaskFillIndexerCommandsQueue::handleMessage(MessageIndexingInterrupted*  /*message*/)
{
	aidkit::access([](auto &indexerCommandProvider, auto &indexerCommandManager, auto &filePathQueue)
	{
		LOG_INFO("Discarding remaining " + std::to_string(indexerCommandProvider->size() + indexerCommandManager.indexerCommandCount()) + " indexer commands.");

		std::queue<FilePath> empty;
		std::swap(filePathQueue, empty);

		indexerCommandProvider->clear();
		indexerCommandManager.clearIndexerCommands();

		LOG_INFO("Remaining: " + std::to_string(indexerCommandProvider->size() + indexerCommandManager.indexerCommandCount()) + ".");
	}, m_indexerCommandProvider, m_indexerCommandManager, m_filePathQueue);
}

bool TaskFillIndexerCommandsQueue::fillCommandQueue()
{
	return aidkit::access([this](auto &indexerCommandProvider, auto &indexerCommandManager, auto &filePathQueue)
	{
		size_t refillAmount = m_maximumQueueSize - indexerCommandManager.indexerCommandCount();
		if (refillAmount == 0)
		{
			return false;
		}
		std::vector<std::shared_ptr<IndexerCommand>> commands;

		while (!indexerCommandProvider->empty() && commands.size() < refillAmount)
		{
			if (!filePathQueue.empty())
			{
				commands.push_back(indexerCommandProvider->consumeCommandForSourceFilePath(filePathQueue.front()));
				filePathQueue.pop();
			}
			else
			{
				commands.push_back(indexerCommandProvider->consumeCommand());
			}
		}

		if (commands.size())
		{
			indexerCommandManager.pushIndexerCommands(commands);
			return true;
		}

		return false;
	}, m_indexerCommandProvider, m_indexerCommandManager, m_filePathQueue);
}
