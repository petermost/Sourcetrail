#include "TaskExecuteCustomCommands.h"

#include "ApplicationSettings.h"
#include "Blackboard.h"
#include "DialogView.h"
#include "ElementComponentKind.h"
#include "FileSystem.h"
#include "IndexerCommandCustom.h"
#include "IndexerCommandProvider.h"
#include "MessageErrorCountClear.h"
#include "MessageErrorCountUpdate.h"
#include "MessageIndexingStatus.h"
#include "MessageShowStatus.h"
#include "MessageStatus.h"
#include "PersistentStorage.h"
#include "SourceLocationCollection.h"
#include "SourceLocationFile.h"
#include "TextAccess.h"
#include "utility.h"
#include "utilityApp.h"
#include "utilityFile.h"
#include "utilityString.h"

using namespace utility;

TaskExecuteCustomCommands::TaskExecuteCustomCommands(
	std::unique_ptr<IndexerCommandProvider> indexerCommandProvider,
	std::shared_ptr<PersistentStorage> storage,
	std::shared_ptr<DialogView> dialogView,
	size_t indexerThreadCount,
	const FilePath& projectDirectory)
	: m_indexerCommandProvider(std::move(indexerCommandProvider))
	, m_storage(storage)
	, m_dialogView(dialogView)
	, m_indexerThreadCount(indexerThreadCount)
	, m_projectDirectory(projectDirectory)
	, m_indexerCommandCount(m_indexerCommandProvider->size())

{
}

void TaskExecuteCustomCommands::doEnter(std::shared_ptr<Blackboard>  /*blackboard*/)
{
	m_dialogView->hideUnknownProgressDialog();
	m_start = TimeStamp::now();

	if (m_indexerCommandProvider)
	{
		for (const FilePath& sourceFilePath:
			 utility::partitionFilePathsBySize(m_indexerCommandProvider->getAllSourceFilePaths(), 2))
		{
			if (std::shared_ptr<IndexerCommandCustom> indexerCommand =
					std::dynamic_pointer_cast<IndexerCommandCustom>(
						m_indexerCommandProvider->consumeCommandForSourceFilePath(sourceFilePath)))
			{
				if (m_targetDatabaseFilePath.empty())
				{
					m_targetDatabaseFilePath = indexerCommand->getDatabaseFilePath();
				}

				if (indexerCommand->getRunInParallel())
				{
					m_parallelCommands.push_back(indexerCommand);
				}
				else
				{
					m_serialCommands.push_back(indexerCommand);
				}
			}
		}
		// reverse because we pull elements from the back of these vectors
		std::reverse(m_parallelCommands.begin(), m_parallelCommands.end());
		std::reverse(m_serialCommands.begin(), m_serialCommands.end());
	}
}

Task::TaskState TaskExecuteCustomCommands::doUpdate(std::shared_ptr<Blackboard> blackboard)
{
	if (m_interrupted)
	{
		return STATE_SUCCESS;
	}

	m_dialogView->updateCustomIndexingDialog(0, 0, m_indexerCommandProvider->size(), {});

	std::vector<std::shared_ptr<std::thread>> indexerThreads;
	for (size_t i = 1 /*this method is counting as the first thread*/; i < m_indexerThreadCount; i++)
	{
		indexerThreads.push_back(std::make_shared<std::thread>(
			&TaskExecuteCustomCommands::executeParallelIndexerCommands,
			this,
			static_cast<int>(i),
			blackboard));
	}

	while (!m_interrupted && !m_serialCommands.empty())
	{
		std::shared_ptr<IndexerCommandCustom> indexerCommand = m_serialCommands.back();
		m_serialCommands.pop_back();
		runIndexerCommand(indexerCommand, blackboard, m_storage);
	}

	executeParallelIndexerCommands(0, blackboard);

	for (std::shared_ptr<std::thread> indexerThread: indexerThreads)
	{
		indexerThread->join();
	}
	indexerThreads.clear();

	// clear errors here, because otherwise injecting into the main storage will show them twice
	MessageErrorCountClear().dispatch();

	{
		PersistentStorage targetStorage(m_targetDatabaseFilePath, FilePath());
		targetStorage.setup();
		targetStorage.setMode(SqliteIndexStorage::STORAGE_MODE_WRITE);
		targetStorage.buildCaches();
		for (const FilePath& sourceDatabaseFilePath: m_sourceDatabaseFilePaths)
		{
			{
				PersistentStorage sourceStorage(sourceDatabaseFilePath, FilePath());
				sourceStorage.setMode(SqliteIndexStorage::STORAGE_MODE_READ);
				sourceStorage.buildCaches();
				targetStorage.inject(&sourceStorage);
			}
			FileSystem::remove(sourceDatabaseFilePath);
		}
	}

	return STATE_SUCCESS;
}

void TaskExecuteCustomCommands::doExit(std::shared_ptr<Blackboard> blackboard)
{
	m_storage.reset();
	const float duration = static_cast<float>(TimeStamp::durationSeconds(m_start));
	blackboard->update<float>(
		"index_time", [duration](float currentDuration) { return currentDuration + duration; });
}

void TaskExecuteCustomCommands::doReset(std::shared_ptr<Blackboard>  /*blackboard*/) {}

void TaskExecuteCustomCommands::handleMessage(MessageIndexingInterrupted*  /*message*/)
{
	LOG_INFO("Interrupting custom command execution.");

	m_interrupted = true;

	m_dialogView->showUnknownProgressDialog(
		"Interrupting Indexing", "Waiting for running\ncommand to finish");
}

void TaskExecuteCustomCommands::executeParallelIndexerCommands(
	int threadId, std::shared_ptr<Blackboard> blackboard)
{
	std::shared_ptr<PersistentStorage> storage;
	while (!m_interrupted)
	{
		std::shared_ptr<IndexerCommandCustom> indexerCommand;
		{
			std::lock_guard<std::mutex> lock(m_parallelCommandsMutex);
			if (m_parallelCommands.empty())
			{
				return;
			}
			indexerCommand = m_parallelCommands.back();
			m_parallelCommands.pop_back();
		}

		if (threadId == 0)
		{
			storage = m_storage;
		}
		else
		{
			FilePath databaseFilePath = indexerCommand->getDatabaseFilePath();
			databaseFilePath = databaseFilePath.getParentDirectory().concatenate(
				databaseFilePath.fileName() + "_thread" + std::to_string(threadId));

			bool databaseFilePathKnown = true;
			{
				std::lock_guard<std::mutex> lock(m_sourceDatabaseFilePathsMutex);
				if (m_sourceDatabaseFilePaths.find(databaseFilePath) ==
					m_sourceDatabaseFilePaths.end())
				{
					m_sourceDatabaseFilePaths.insert(databaseFilePath);
					databaseFilePathKnown = false;
				}
			}

			if (!databaseFilePathKnown)
			{
				if (databaseFilePath.exists())
				{
					LOG_WARNING(
						"Temporary storage \"" + databaseFilePath.str() +
						"\" already exists on file system. File will be removed to avoid "
						"conflicts.");
					FileSystem::remove(databaseFilePath);
				}
				storage = std::make_shared<PersistentStorage>(databaseFilePath, FilePath());
				storage->setup();
				storage->setMode(SqliteIndexStorage::STORAGE_MODE_WRITE);
				storage->buildCaches();
			}

			indexerCommand->setDatabaseFilePath(databaseFilePath);
		}

		runIndexerCommand(indexerCommand, blackboard, storage);
	}
}

void TaskExecuteCustomCommands::runIndexerCommand(
	std::shared_ptr<IndexerCommandCustom> indexerCommand,
	std::shared_ptr<Blackboard> blackboard,
	std::shared_ptr<PersistentStorage> storage)
{
	if (indexerCommand)
	{
		int indexedSourceFileCount = 0;
		blackboard->get("indexed_source_file_count", indexedSourceFileCount);

		const FilePath sourcePath = indexerCommand->getSourceFilePath();

		m_dialogView->updateCustomIndexingDialog(
			indexedSourceFileCount + 1, indexedSourceFileCount, m_indexerCommandCount, {sourcePath});
		MessageIndexingStatus(true, indexedSourceFileCount * 100 / m_indexerCommandCount).dispatch();

		const std::string command = indexerCommand->getCommand();
		const std::vector<std::string> arguments = indexerCommand->getArguments();

		LOG_INFO("Start processing command \"" +	command + " " + utility::join(arguments, " ") + "\"");

		const ErrorCountInfo previousErrorCount = storage ? storage->getErrorCount()
														  : ErrorCountInfo();

		LOG_INFO("Starting to index");
		const utility::ProcessOutput out = utility::executeProcess(
			command, arguments, m_projectDirectory, false, INFINITE_TIMEOUT, true);
		LOG_INFO("Finished indexing");

		if (storage)
		{
			std::vector<ErrorInfo> errors = storage->getErrorInfos();
			const ErrorCountInfo currentErrorCount(errors);
			if (currentErrorCount.total > previousErrorCount.total)
			{
				const ErrorCountInfo diff(
					currentErrorCount.total - previousErrorCount.total,
					currentErrorCount.fatal - previousErrorCount.fatal);

				ErrorCountInfo errorCount;	  // local copy to release lock early
				{
					std::lock_guard<std::mutex> lock(m_errorCountMutex);
					m_errorCount.total += diff.total;
					m_errorCount.fatal += diff.fatal;
					errorCount = m_errorCount;
				}

				errors.erase(errors.begin(), errors.begin() + previousErrorCount.total);
				MessageErrorCountUpdate(errorCount, errors).dispatch();
			}
		}

		if (out.exitCode == 0 && out.error.empty())
		{
			std::string message = "Process returned successfully.\n";
			LOG_INFO(message);
		}
		else
		{
			std::string statusText = "command \"" + indexerCommand->getCommand() + " " +
				utility::join(arguments, " ") + "\" returned";
			if (out.exitCode != 0)
			{
				statusText += " code \"" + std::to_string(out.exitCode) + "\"";
			}
			if (!out.error.empty())
			{
				statusText += " with message \"" + out.error + "\"";
			}
			statusText += ".";

			LOG_ERROR(statusText);
			MessageShowStatus().dispatch();
			MessageStatus(statusText, true, false, true).dispatch();
		}

		indexedSourceFileCount++;
		blackboard->update<int>("indexed_source_file_count", [=](int count) { return count + 1; });
	}
}
