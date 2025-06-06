#include "TaskFinishParsing.h"

#include "Blackboard.h"
#include "DialogView.h"
#include "MessageIndexingFinished.h"
#include "MessageIndexingStatus.h"
#include "MessageStatus.h"
#include "PersistentStorage.h"
#include "TimeStamp.h"

TaskFinishParsing::TaskFinishParsing(
	std::shared_ptr<PersistentStorage> storage, std::shared_ptr<DialogView> dialogView)
	: m_storage(storage), m_dialogView(dialogView)
{
}

void TaskFinishParsing::terminate()
{
	m_dialogView->clearDialogs();

	MessageStatus("An unknown exception was thrown during indexing.", true, false).dispatch();
	MessageIndexingFinished().dispatch();
}

void TaskFinishParsing::doEnter(std::shared_ptr<Blackboard>  /*blackboard*/)
{
	m_storage->setMode(SqliteIndexStorage::STORAGE_MODE_READ);
}

Task::TaskState TaskFinishParsing::doUpdate(std::shared_ptr<Blackboard> blackboard)
{
	TimeStamp start = TimeStamp::now();

	m_dialogView->showUnknownProgressDialog("Finish Indexing", "Optimizing database");
	m_storage->optimizeMemory();
	m_dialogView->hideUnknownProgressDialog();

	double time = TimeStamp::durationSeconds(start);

	if (blackboard->exists("clear_time"))
	{
		float clearTime = 0;
		blackboard->get("clear_time", clearTime);
		time += clearTime;
	}

	if (blackboard->exists("index_time"))
	{
		float indexTime = 0;
		blackboard->get("index_time", indexTime);
		time += indexTime;
	}

	int indexedSourceFileCount = 0;
	blackboard->get("indexed_source_file_count", indexedSourceFileCount);

	int sourceFileCount = 0;
	blackboard->get("source_file_count", sourceFileCount);

	bool interruptedIndexing = false;
	blackboard->get("interrupted_indexing", interruptedIndexing);

	bool shallowIndexing = false;
	blackboard->get("shallow_indexing", shallowIndexing);

	ErrorCountInfo errorInfo = m_storage->getErrorCount();

	std::string status;
	status += "Finished indexing: ";
	status += std::to_string(indexedSourceFileCount) + "/" + std::to_string(sourceFileCount) +
		" source files indexed; ";
	status += TimeStamp::secondsToString(time);
	status += "; " + std::to_string(errorInfo.total) + " error" +
		(errorInfo.total != 1 ? "s" : "");
	if (errorInfo.fatal > 0)
	{
		status += " (" + std::to_string(errorInfo.fatal) + " fatal)";
	}
	MessageStatus(status, false, false).dispatch();

	StorageStats stats = m_storage->getStorageStats();
	DatabasePolicy policy = m_dialogView->finishedIndexingDialog(
		indexedSourceFileCount,
		sourceFileCount,
		stats.completedFileCount,
		stats.fileCount,
		static_cast<float>(time),
		errorInfo,
		interruptedIndexing,
		shallowIndexing);

	MessageIndexingStatus(false).dispatch();

	if (policy == DATABASE_POLICY_KEEP)
	{
		blackboard->set("keep_database", true);
	}
	else if (policy == DATABASE_POLICY_DISCARD)
	{
		blackboard->set("discard_database", true);
	}
	else if (policy == DATABASE_POLICY_REFRESH)
	{
		blackboard->set("keep_database", true);
		blackboard->set("refresh_database", true);
	}

	return STATE_SUCCESS;
}

void TaskFinishParsing::doExit(std::shared_ptr<Blackboard>  /*blackboard*/)
{
	m_storage.reset();
}

void TaskFinishParsing::doReset(std::shared_ptr<Blackboard>  /*blackboard*/) {}
