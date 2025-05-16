#include "Project.h"

#include "ApplicationSettings.h"
#include "CombinedIndexerCommandProvider.h"
#include "DialogView.h"
#include "IndexerCommand.h"
#include "IndexerCommandCustom.h"
#include "PersistentStorage.h"
#include "ProjectSettings.h"
#include "RefreshInfoGenerator.h"
#include "SourceGroup.h"
#include "SourceGroupFactory.h"
#include "SourceGroupStatusType.h"
#include "StorageCache.h"
#include "StorageProvider.h"
#include "TaskBuildIndex.h"
#include "TaskCleanStorage.h"
#include "TaskExecuteCustomCommands.h"
#include "TaskFillIndexerCommandQueue.h"
#include "TaskFinishParsing.h"
#include "TaskInjectStorage.h"
#include "TaskMergeStorages.h"
#include "TaskParseWrapper.h"

#include "FilePath.h"
#include "FileSystem.h"
#include "MessageErrorCountClear.h"
#include "MessageIndexingFinished.h"
#include "MessageIndexingShowDialog.h"
#include "MessageIndexingStarted.h"
#include "MessageIndexingStatus.h"
#include "MessageRefresh.h"
#include "MessageStatus.h"
#include "TabIds.h"
#include "TaskDecoratorRepeat.h"
#include "TaskFindKeyOnBlackboard.h"
#include "TaskGroupParallel.h"
#include "TaskGroupSelector.h"
#include "TaskGroupSequence.h"
#include "TaskLambda.h"
#include "TaskReturnSuccessIf.h"
#include "TaskSetValue.h"
#include "TextAccess.h"
#include "utility.h"
#include "utilityApp.h"
#include "utilityFile.h"
#include "utilityString.h"

Project::Project(
	std::shared_ptr<ProjectSettings> settings,
	StorageCache* storageCache,
	const std::string& appUUID,
	bool hasGUI)
	: m_settings(settings)
	, m_storageCache(storageCache)
	, m_appUUID(appUUID)
	, m_hasGUI(hasGUI)
{
}

Project::~Project() = default;

FilePath Project::getProjectSettingsFilePath() const
{
	return m_settings->getFilePath();
}

std::string Project::getDescription() const
{
	return m_settings->getDescription();
}

bool Project::isLoaded() const
{
	switch (m_state)
	{
	case PROJECT_STATE_EMPTY:
	case PROJECT_STATE_LOADED:
	case PROJECT_STATE_OUTDATED:
	case PROJECT_STATE_NEEDS_MIGRATION:
		return true;

	default:
		break;
	}

	return false;
}

bool Project::isIndexing() const
{
	return m_refreshStage == RefreshStageType::INDEXING;
}

bool Project::settingsEqualExceptNameAndLocation(const ProjectSettings& otherSettings) const
{
	return m_settings->equalsExceptNameAndLocation(otherSettings);
}

void Project::setStateOutdated()
{
	if (m_state == PROJECT_STATE_LOADED)
	{
		m_state = PROJECT_STATE_OUTDATED;
	}
}

void Project::load(std::shared_ptr<DialogView> dialogView)
{
	if (m_refreshStage != RefreshStageType::NONE)
	{
		MessageStatus("Cannot load another project while indexing.", true, false).dispatch();
		return;
	}

	m_storageCache->clear();
	m_storageCache->setSubject(
		std::weak_ptr<StorageAccess>());	// TODO: check if this is really required.

	if (!m_settings->reload())
	{
		return;
	}

	const FilePath projectSettingsPath = m_settings->getFilePath();
	const FilePath dbPath = m_settings->getDBFilePath();
	const FilePath tempDbPath = m_settings->getTempDBFilePath();
	const FilePath bookmarkDbPath = m_settings->getBookmarkDBFilePath();

	{
		if (tempDbPath.exists())
		{
			if (dbPath.exists())
			{
				if (dialogView->confirm(
						"Sourcetrail has been closed unexpectedly while indexing this project. "
						"You can either choose to keep "
						"the data that has already been indexed or discard that data and restore "
						"the state of your project "
						"before indexing?",
						{"Keep and Continue", "Discard and Restore"}) == 0)
				{
					LOG_INFO("Switching to temporary indexing data on user's decision");
					if (!swapToTempStorageFile(dbPath, tempDbPath, dialogView))
					{
						m_state = PROJECT_STATE_NOT_LOADED;
						MessageStatus("Unable to load project", true, false).dispatch();
						return;
					}
				}
				else
				{
					LOG_INFO("Discarding temporary indexing data on user's decision");
					FileSystem::remove(tempDbPath);
				}
			}
			else
			{
				LOG_INFO(
					"Switching to temporary indexing data because no other persistent data was "
					"found");
				FileSystem::rename(tempDbPath, dbPath);
			}
		}
	}

	m_storage = std::make_shared<PersistentStorage>(dbPath, bookmarkDbPath);

	bool canLoad = false;

	if (m_settings->needMigration())
	{
		m_state = PROJECT_STATE_NEEDS_MIGRATION;

		if (!m_storage->isEmpty() && !m_storage->isIncompatible())
		{
			canLoad = true;
		}
	}
	else if (m_storage->isEmpty())
	{
		m_state = PROJECT_STATE_EMPTY;
	}
	else if (m_storage->isIncompatible())
	{
		m_state = PROJECT_STATE_OUTVERSIONED;
	}
	else
	{
		ProjectSettings storedSettings;
		if (storedSettings.loadFromString(m_storage->getProjectSettingsText()) &&
			m_settings->equalsExceptNameAndLocation(storedSettings))
		{
			m_state = PROJECT_STATE_LOADED;
			canLoad = true;
		}
		else
		{
			m_state = PROJECT_STATE_OUTDATED;
			canLoad = true;
		}
	}

	try
	{
		m_storage->setup();
	}
	catch (...)
	{
		LOG_ERROR("Exception has been encountered while loading the project.");

		canLoad = false;
		m_state = PROJECT_STATE_DB_CORRUPTED;
	}

	m_sourceGroups = SourceGroupFactory::getInstance()->createSourceGroups(
		m_settings->getAllSourceGroupSettings());

	if (canLoad)
	{
		m_storage->setMode(SqliteIndexStorage::STORAGE_MODE_READ);
		m_storage->buildCaches();
		m_storageCache->setSubject(m_storage);

		if (m_hasGUI)
		{
			MessageIndexingFinished().dispatch();
		}
		MessageStatus("Finished Loading", false, false).dispatch();
	}
	else
	{
		switch (m_state)
		{
		case PROJECT_STATE_NEEDS_MIGRATION:
			MessageStatus(
				"Project could not be loaded and needs to be re-indexed after automatic migration "
				"to latest "
				"version.",
				false,
				false)
				.dispatch();
			break;
		case PROJECT_STATE_EMPTY:
			MessageStatus(
				"Project could not load any symbols because the index database is empty. Please "
				"re-index the "
				"project.",
				false,
				false)
				.dispatch();
			break;
		case PROJECT_STATE_OUTVERSIONED:
			MessageStatus(
				"Project could not be loaded because the indexed data format is incompatible to "
				"the current "
				"version of Sourcetrail. Please re-index the project.",
				false,
				false)
				.dispatch();
			break;
		default:
			MessageStatus("Project could not be loaded.", false, false).dispatch();
		}
	}

	if (m_state != PROJECT_STATE_LOADED && m_hasGUI)
	{
		MessageRefresh().dispatch();
	}
}

void Project::refresh(
	std::shared_ptr<DialogView> dialogView, RefreshMode refreshMode, bool shallowIndexingRequested)
{
	if (m_refreshStage != RefreshStageType::NONE)
	{
		return;
	}

	if (m_state == PROJECT_STATE_NOT_LOADED)
	{
		return;
	}

	bool needsFullRefresh = false;
	bool fullRefresh = false;
	std::string question;

	switch (m_state)
	{
	case PROJECT_STATE_EMPTY:
		needsFullRefresh = true;
		break;

	case PROJECT_STATE_LOADED:
		break;

	case PROJECT_STATE_OUTDATED:
		question =
			"The project file was changed after the last indexing. The project needs to get fully "
			"reindexed to "
			"reflect the current project state. Alternatively you can also choose to just reindex "
			"updated or "
			"incomplete files. Do you want to reindex the project?";
		fullRefresh = true;
		break;

	case PROJECT_STATE_OUTVERSIONED:
		question =
			"This project was indexed with a different version of Sourcetrail. It needs to be "
			"fully reindexed to "
			"be used with this version of Sourcetrail. Do you want to reindex the project?";
		needsFullRefresh = true;
		break;

	case PROJECT_STATE_NEEDS_MIGRATION:
		question =
			"This project was created with a different version and uses an old project file "
			"format. "
			"The project can still be opened and used with this version, but needs to be fully "
			"reindexed. "
			"Do you want Sourcetrail to update the project file and reindex the project?";
		needsFullRefresh = true;
		break;

	case PROJECT_STATE_DB_CORRUPTED:
		question =
			"There was a problem loading the index of this project. The project needs to get "
			"fully reindexed. "
			"Do you want to reindex the project?";
		needsFullRefresh = true;
		break;

	default:
		break;
	}

	if (question.size() && m_hasGUI)
	{
		if (dialogView->confirm(question, {"Reindex", "Cancel"}) == 1)
		{
			return;
		}
	}

	if (ApplicationSettings::getInstance()->getLoggingEnabled() &&
		ApplicationSettings::getInstance()->getVerboseIndexerLoggingEnabled() && m_hasGUI)
	{
		if (dialogView->confirm(
				"Warning: You are about to index your project with the \"verbose indexer "
				"logging\" setting "
				"enabled. This will cause a significant slowdown in indexing performance. Do you "
				"want to proceed?",
				{"Yes", "No"}) == 1)
		{
			return;
		}
	}

	m_refreshStage = RefreshStageType::REFRESHING;

	if (m_state == PROJECT_STATE_NEEDS_MIGRATION)
	{
		m_settings->migrate();
	}

	m_settings->reload();

	m_sourceGroups = SourceGroupFactory::getInstance()->createSourceGroups(
		m_settings->getAllSourceGroupSettings());
	for (const std::shared_ptr<SourceGroup>& sourceGroup: m_sourceGroups)
	{
		if (sourceGroup->getStatus() == SourceGroupStatusType::ENABLED && !sourceGroup->prepareIndexing())
		{
			m_refreshStage = RefreshStageType::NONE;
			return;
		}
	}

	if (needsFullRefresh || fullRefresh)
	{
		refreshMode = RefreshMode::ALL_FILES;
	}
	else if (refreshMode == RefreshMode::NONE)
	{
		refreshMode = RefreshMode::UPDATED_FILES;
	}

	bool allowsShallowIndexing = false;
	for (const std::shared_ptr<SourceGroup>& sourceGroup: m_sourceGroups)
	{
		if (sourceGroup->getStatus() == SourceGroupStatusType::ENABLED &&
			sourceGroup->allowsShallowIndexing())
		{
			allowsShallowIndexing = true;
			break;
		}
	}

	const bool useShallowIndexing = allowsShallowIndexing && shallowIndexingRequested;

	if (m_hasGUI)
	{
		std::vector<RefreshMode> enabledModes = {RefreshMode::ALL_FILES};
		if (!needsFullRefresh)
		{
			enabledModes.insert(
				enabledModes.end(), {RefreshMode::UPDATED_FILES, RefreshMode::UPDATED_AND_INCOMPLETE_FILES});
		}

		dialogView->startIndexingDialog(
			this,
			enabledModes,
			refreshMode,
			allowsShallowIndexing,
			useShallowIndexing,
			[this, dialogView](const RefreshInfo& info) { buildIndex(info, dialogView); },
			[this]() { m_refreshStage = RefreshStageType::NONE; });
	}
	else
	{
		RefreshInfo info = getRefreshInfo(refreshMode);
		info.shallow = useShallowIndexing;
		buildIndex(info, dialogView);
	}
}

RefreshInfo Project::getRefreshInfo(RefreshMode mode) const
{
	switch (mode)
	{
	case RefreshMode::NONE:
		return RefreshInfo();

	case RefreshMode::UPDATED_FILES:
		return RefreshInfoGenerator::getRefreshInfoForUpdatedFiles(m_sourceGroups, m_storage);

	case RefreshMode::UPDATED_AND_INCOMPLETE_FILES:
		return RefreshInfoGenerator::getRefreshInfoForIncompleteFiles(m_sourceGroups, m_storage);

	case RefreshMode::ALL_FILES:
	default:
		return RefreshInfoGenerator::getRefreshInfoForAllFiles(m_sourceGroups);
	}
}

void Project::buildIndex(RefreshInfo info, std::shared_ptr<DialogView> dialogView)
{
	if (m_refreshStage == RefreshStageType::INDEXING)
	{
		MessageStatus("Cannot refresh project while indexing.", true, false).dispatch();
		return;
	}

	{
		std::string message;
		if (info.mode != RefreshMode::ALL_FILES && info.filesToClear.empty() && info.filesToIndex.empty())
		{
			message = "Nothing to refresh, all files are up-to-date.";
		}
		else if (m_sourceGroups.empty())
		{
			message = "Nothing to refresh, no Source Groups loaded.";
		}

		if (!message.empty())
		{
			if (m_hasGUI)
			{
				dialogView->clearDialogs();
			}
			else
			{
				MessageIndexingFinished().dispatch();
			}

			MessageStatus(message).dispatch();
			m_refreshStage = RefreshStageType::NONE;
			return;
		}
	}

	if (info.mode != RefreshMode::ALL_FILES &&
		(info.filesToClear.size() || info.nonIndexedFilesToClear.size()))
	{
		for (const std::shared_ptr<SourceGroup>& sourceGroup: m_sourceGroups)
		{
			if (sourceGroup->getStatus() == SourceGroupStatusType::ENABLED &&
				!sourceGroup->allowsPartialClearing())
			{
				bool abortIndexing = false;
				for (const FilePath& sourcePath:
					 utility::concat(info.filesToClear, info.nonIndexedFilesToClear))
				{
					if (sourceGroup->containsSourceFilePath(sourcePath))
					{
						abortIndexing = true;
						break;
					}
				}

				if (abortIndexing)
				{
					if (m_hasGUI &&
						dialogView->confirm(
							"<p>This project contains a source group of type \"" +
								utility::decodeFromUtf8(
									sourceGroupTypeToString(sourceGroup->getType())) +
								"\" that cannot be partially cleared. Do you want to re-index the "
								"whole project instead?</p>",
							{"Full Re-Index", "Cancel"}) == 1)
					{
						MessageStatus("Cannot partially clear project. Indexing aborted.").dispatch();
						m_refreshStage = RefreshStageType::NONE;
						dialogView->clearDialogs();
						return;
					}
					else
					{
						const bool shallow = info.shallow;
						info = getRefreshInfo(RefreshMode::ALL_FILES);
						info.shallow = shallow;
					}
				}

				break;
			}
		}
	}

	MessageStatus("Preparing Indexing", false, true).dispatch();
	MessageErrorCountClear().dispatch();

	dialogView->showUnknownProgressDialog("Preparing Indexing", "Setting up Indexers");
	MessageIndexingStatus(true, 0).dispatch();

	m_storageCache->clear();
	m_storageCache->setSubject(m_storage);

	const FilePath indexDbFilePath = m_settings->getDBFilePath();
	const FilePath tempIndexDbFilePath = m_settings->getTempDBFilePath();

	if (info.mode != RefreshMode::ALL_FILES)
	{
		// store the indexed data into the temp db but keep the current state to allow browsing
		// while indexing
		FileSystem::copyFile(indexDbFilePath, tempIndexDbFilePath);
	}

	std::shared_ptr<PersistentStorage> tempStorage = std::make_shared<PersistentStorage>(
		tempIndexDbFilePath, m_storage->getBookmarkDbFilePath());
	tempStorage->setup();

	std::shared_ptr<TaskGroupSequence> taskSequential = std::make_shared<TaskGroupSequence>();

	if (info.mode != RefreshMode::ALL_FILES &&
		(info.filesToClear.size() || info.nonIndexedFilesToClear.size()))
	{
		taskSequential->addTask(std::make_shared<TaskCleanStorage>(
			tempStorage,
			dialogView,
			utility::toVector(utility::concat(info.filesToClear, info.nonIndexedFilesToClear)),
			info.mode == RefreshMode::UPDATED_AND_INCOMPLETE_FILES));
	}

	tempStorage->setProjectSettingsText(
		TextAccess::createFromFile(getProjectSettingsFilePath())->getText());
	tempStorage->updateVersion();

	std::unique_ptr<CombinedIndexerCommandProvider> indexerCommandProvider =
		std::make_unique<CombinedIndexerCommandProvider>();
	std::unique_ptr<CombinedIndexerCommandProvider> customIndexerCommandProvider =
		std::make_unique<CombinedIndexerCommandProvider>();
	for (const std::shared_ptr<SourceGroup>& sourceGroup: m_sourceGroups)
	{
		if (sourceGroup->getStatus() == SourceGroupStatusType::ENABLED)
		{
			if (sourceGroup->getType() == SourceGroupType::CUSTOM_COMMAND)
			{
				customIndexerCommandProvider->addProvider(
					sourceGroup->getIndexerCommandProvider(info));
			}
#if BUILD_PYTHON_LANGUAGE_PACKAGE
			else if (sourceGroup->getType() == SourceGroupType::PYTHON_EMPTY)
			{
				customIndexerCommandProvider->addProvider(
					sourceGroup->getIndexerCommandProvider(info));
			}
#endif	  // BUILD_PYTHON_LANGUAGE_PACKAGE
			else
			{
				indexerCommandProvider->addProvider(sourceGroup->getIndexerCommandProvider(info));
			}
		}
	}

	size_t sourceFileCount = indexerCommandProvider->size() + customIndexerCommandProvider->size();

	taskSequential->addTask(std::make_shared<TaskSetValue<bool>>("shallow_indexing", info.shallow));
	taskSequential->addTask(std::make_shared<TaskSetValue<int>>(
		"source_file_count", static_cast<int>(sourceFileCount)));
	taskSequential->addTask(std::make_shared<TaskSetValue<int>>("indexed_source_file_count", 0));
	taskSequential->addTask(std::make_shared<TaskSetValue<bool>>("interrupted_indexing", false));
	taskSequential->addTask(std::make_shared<TaskSetValue<float>>("index_time", 0.0f));

	int indexerThreadCount = ApplicationSettings::getInstance()->getIndexerThreadCount();
	if (indexerThreadCount <= 0)
	{
		indexerThreadCount = utility::getIdealThreadCount();
		if (indexerThreadCount <= 0)
		{
			indexerThreadCount = 4;	   // setting to some fallback value
		}
	}

	if (!indexerCommandProvider->empty())
	{
		const int adjustedIndexerThreadCount = std::min<int>(
			indexerThreadCount, static_cast<int>(indexerCommandProvider->size()));

		std::shared_ptr<StorageProvider> storageProvider = std::make_shared<StorageProvider>();
		// add tasks for setting some variables on the blackboard that are used during indexing
		taskSequential->addTask(
			std::make_shared<TaskSetValue<bool>>("indexer_threads_started", false));
		taskSequential->addTask(
			std::make_shared<TaskSetValue<bool>>("indexer_threads_stopped", false));
		taskSequential->addTask(
			std::make_shared<TaskSetValue<bool>>("indexer_command_queue_started", false));
		taskSequential->addTask(
			std::make_shared<TaskSetValue<bool>>("indexer_command_queue_stopped", false));

		std::shared_ptr<TaskGroupSequence> preIndexTasks = std::make_shared<TaskGroupSequence>();
		taskSequential->addTask(preIndexTasks);
		for (const std::shared_ptr<SourceGroup>& sourceGroup: m_sourceGroups)
		{
			if (sourceGroup->getStatus() == SourceGroupStatusType::ENABLED)
			{
				preIndexTasks->addTask(sourceGroup->getPreIndexTask(storageProvider, dialogView));
			}
		}

		std::shared_ptr<TaskParseWrapper> taskParserWrapper = std::make_shared<TaskParseWrapper>(
			tempStorage, dialogView);
		taskSequential->addTask(taskParserWrapper);

		std::shared_ptr<TaskGroupParallel> taskParallelIndexing =
			std::make_shared<TaskGroupParallel>();
		taskParserWrapper->setTask(taskParallelIndexing);

		// add task for refilling the indexer command queue
		taskParallelIndexing->addTask(std::make_shared<TaskFillIndexerCommandsQueue>(
			m_appUUID, std::move(indexerCommandProvider), 20));

		// add task for indexing
		bool multiProcess = ApplicationSettings::getInstance()->getMultiProcessIndexingEnabled() &&
			hasCxxSourceGroup();
		taskParallelIndexing->addChildTasks(std::make_shared<TaskGroupSequence>()->addChildTasks(
			// block until there are indexer commands to process
			std::make_shared<TaskDecoratorRepeat>(
				TaskDecoratorRepeat::CONDITION_WHILE_SUCCESS, Task::STATE_SUCCESS, 25)
				->addChildTask(std::make_shared<TaskReturnSuccessIf<bool>>(
					"indexer_command_queue_started", TaskReturnSuccessIf<bool>::CONDITION_EQUALS, false)),
			std::make_shared<TaskBuildIndex>(
				adjustedIndexerThreadCount, storageProvider, dialogView, m_appUUID, multiProcess)));

		// add task for merging the intermediate storages
		taskParallelIndexing->addTask(std::make_shared<TaskGroupSequence>()->addChildTasks(
			// block until there are indexers running
			std::make_shared<TaskDecoratorRepeat>(
				TaskDecoratorRepeat::CONDITION_WHILE_SUCCESS, Task::STATE_SUCCESS, 25)
				->addChildTask(std::make_shared<TaskReturnSuccessIf<bool>>(
					"indexer_threads_started", TaskReturnSuccessIf<bool>::CONDITION_EQUALS, false)),
			// merge until all indexers stopped and nothing left to merge
			std::make_shared<TaskDecoratorRepeat>(
				TaskDecoratorRepeat::CONDITION_WHILE_SUCCESS, Task::STATE_SUCCESS, 250)
				->addChildTask(std::make_shared<TaskGroupSelector>()->addChildTasks(
					std::make_shared<TaskMergeStorages>(storageProvider),
					std::make_shared<TaskReturnSuccessIf<bool>>(
						"indexer_threads_stopped",
						TaskReturnSuccessIf<bool>::CONDITION_EQUALS,
						false)))));

		// add task for injecting the intermediate storages into the persistent storage
		taskParallelIndexing->addTask(std::make_shared<TaskGroupSequence>()->addChildTasks(
			// block until there are indexers running
			std::make_shared<TaskDecoratorRepeat>(
				TaskDecoratorRepeat::CONDITION_WHILE_SUCCESS, Task::STATE_SUCCESS, 25)
				->addChildTask(std::make_shared<TaskReturnSuccessIf<bool>>(
					"indexer_threads_started", TaskReturnSuccessIf<bool>::CONDITION_EQUALS, false)),
			std::make_shared<TaskDecoratorRepeat>(
				TaskDecoratorRepeat::CONDITION_WHILE_SUCCESS, Task::STATE_SUCCESS, 25)
				->addChildTask(std::make_shared<TaskGroupSelector>()->addChildTasks(
					std::make_shared<TaskInjectStorage>(storageProvider, tempStorage),
					// continuing when indexers still running, even if there are no storages right now.
					std::make_shared<TaskReturnSuccessIf<bool>>(
						"indexer_threads_stopped",
						TaskReturnSuccessIf<bool>::CONDITION_EQUALS,
						false)))));

		// add task that notifies the user of what's going on
		taskSequential->addTask(	// we don't need to hide this dialog again, because it's
									// overridden by other dialogs later on.
			std::make_shared<TaskLambda>([dialogView]() {
				dialogView->showUnknownProgressDialog("Finish Indexing", "Saving\nRemaining Data");
			}));

		// add task that injects the remaining intermediate storages into the persistent storage
		taskSequential->addTask(
			std::make_shared<TaskDecoratorRepeat>(
				TaskDecoratorRepeat::CONDITION_WHILE_SUCCESS, Task::STATE_SUCCESS, 25)
				->addChildTask(std::make_shared<TaskInjectStorage>(storageProvider, tempStorage)));
	}
	else
	{
		dialogView->hideUnknownProgressDialog();
	}

	if (!customIndexerCommandProvider->empty())
	{
		const int adjustedIndexerThreadCount = std::min<int>(
			indexerThreadCount, static_cast<int>(customIndexerCommandProvider->size()));

		taskSequential->addTask(std::make_shared<TaskExecuteCustomCommands>(
			std::move(customIndexerCommandProvider),
			tempStorage,
			dialogView,
			adjustedIndexerThreadCount,
			getProjectSettingsFilePath().getParentDirectory()));
	}

	taskSequential->addTask(std::make_shared<TaskFinishParsing>(tempStorage, dialogView));

	taskSequential->addTask(std::make_shared<TaskGroupSelector>()->addChildTasks(
		std::make_shared<TaskGroupSequence>()->addChildTasks(
			std::make_shared<TaskFindKeyOnBlackboard>("keep_database"),
			std::make_shared<TaskLambda>([dialogView, this]() {
				Task::dispatch(TabIds::app(), std::make_shared<TaskLambda>([dialogView, this]() {
								   swapToTempStorage(dialogView);
							   }));
			})),
		std::make_shared<TaskGroupSequence>()->addChildTasks(
			std::make_shared<TaskFindKeyOnBlackboard>("discard_database"),
			std::make_shared<TaskLambda>([this]() {
				Task::dispatch(
					TabIds::app(), std::make_shared<TaskLambda>([this]() { discardTempStorage(); }));
			}))));

	taskSequential->addTask(std::make_shared<TaskLambda>([dialogView, this]() {
		m_refreshStage = RefreshStageType::NONE;
		MessageIndexingFinished().dispatch();
	}));

	taskSequential->addTask(std::make_shared<TaskGroupSelector>()->addChildTasks(
		std::make_shared<TaskGroupSequence>()->addChildTasks(
			std::make_shared<TaskFindKeyOnBlackboard>("refresh_database"),
			std::make_shared<TaskLambda>([dialogView, this]() {
				Task::dispatch(TabIds::app(), std::make_shared<TaskLambda>([dialogView, this]() {
								   MessageIndexingShowDialog().dispatch();
								   MessageRefresh().refreshAll().dispatch();
							   }));
			})),
		std::make_shared<TaskGroupSequence>()->addChildTasks(std::make_shared<TaskLambda>([this]() {
			Task::dispatch(TabIds::app(), std::make_shared<TaskLambda>([this]() {}));
		}))));

	taskSequential->setIsBackgroundTask(true);
	Task::dispatch(TabIds::app(), taskSequential);

	m_refreshStage = RefreshStageType::INDEXING;
	MessageStatus(
		"Starting Indexing: " + std::to_string(sourceFileCount) + " source files", false, true)
		.dispatch();
	MessageIndexingStarted().dispatch();
}

void Project::swapToTempStorage(std::shared_ptr<DialogView> dialogView)
{
	LOG_INFO("Switching to temporary indexing data");

	const FilePath indexDbFilePath = m_settings->getDBFilePath();
	const FilePath tempIndexDbFilePath = m_settings->getTempDBFilePath();
	const FilePath bookmarkDbFilePath = m_settings->getBookmarkDBFilePath();

	m_storage.reset();

	if (!swapToTempStorageFile(indexDbFilePath, tempIndexDbFilePath, dialogView))
	{
		m_state = PROJECT_STATE_NOT_LOADED;
		return;
	}

	m_storage = std::make_shared<PersistentStorage>(indexDbFilePath, bookmarkDbFilePath);
	m_storage->setup();

	// std::shared_ptr<DialogView> dialogView =
	// Application::getInstance()->getDialogView(DialogView::UseCase::INDEXING);
	// dialogView->showUnknownProgressDialog("Finish Indexing", "Building caches");
	m_storage->buildCaches();
	// dialogView->hideUnknownProgressDialog();

	m_storageCache->setSubject(m_storage);
	m_state = PROJECT_STATE_LOADED;
}

bool Project::swapToTempStorageFile(
	const FilePath& indexDbFilePath,
	const FilePath& tempIndexDbFilePath,
	std::shared_ptr<DialogView> dialogView)
{
	try
	{
		FileSystem::remove(indexDbFilePath);
		FileSystem::rename(tempIndexDbFilePath, indexDbFilePath);
	}
	catch (std::exception& /*e*/)
	{
		if (m_hasGUI)
		{
			dialogView->confirm(
				"<p>The old index database file of this project seems to be used by a different "
				"process and cannot "
				"be updated.</p><p>Please close all processes that are using this database and "
				"re-load this project to "
				"apply or discard the changes pending from the current indexer run.</p>");
		}
		return false;
	}
	return true;
}

void Project::discardTempStorage()
{
	const FilePath tempIndexDbPath = m_settings->getTempDBFilePath();
	if (tempIndexDbPath.exists())
	{
		LOG_INFO("Discarding temporary indexing data");
		FileSystem::remove(tempIndexDbPath);
	}
}

bool Project::hasCxxSourceGroup() const
{
#if BUILD_CXX_LANGUAGE_PACKAGE
	for (const std::shared_ptr<SourceGroup>& sourceGroup: m_sourceGroups)
	{
		if (sourceGroup->getStatus() == SourceGroupStatusType::ENABLED)
		{
			if (sourceGroup->getLanguage() == LanguageType::C || sourceGroup->getLanguage() == LanguageType::CXX)
			{
				return true;
			}
		}
	}
#endif	  // BUILD_CXX_LANGUAGE_PACKAGE
	return false;
}
