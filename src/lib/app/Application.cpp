#include "Application.h"

#include "AppPath.h"
#include "ApplicationSettings.h"
#include "ColorScheme.h"
#include "DialogView.h"
#include "FileLogger.h"
#include "FileSystem.h"
#include "GraphViewStyle.h"
#include "IDECommunicationController.h"
#include "LogManager.h"
#include "MainView.h"
#include "MessageFilterErrorCountUpdate.h"
#include "MessageFilterFocusInOut.h"
#include "MessageFilterSearchAutocomplete.h"
#include "MessageQueue.h"
#include "MessageQuitApplication.h"
#include "MessageStatus.h"
#include "NetworkFactory.h"
#include "ProjectSettings.h"
#include "SharedMemoryGarbageCollector.h"
#include "StorageCache.h"
#include "TabIds.h"
#include "TaskManager.h"
#include "TaskScheduler.h"
#include "UserPaths.h"
#include "Version.h"
#include "ViewFactory.h"
#include "logging.h"
#include "tracing.h"
#include "utilityUuid.h"

#include "CppSQLite3.h"

std::shared_ptr<Application> Application::s_instance;
std::string Application::s_uuid;

void Application::createInstance(
	const Version& version, ViewFactory* viewFactory, NetworkFactory* networkFactory)
{
	bool hasGui = (viewFactory != nullptr);

	Version::setApplicationVersion(version);

	if (hasGui)
	{
		GraphViewStyle::setImpl(viewFactory->createGraphStyleImpl());
	}

	loadSettings();

	SharedMemoryGarbageCollector* collector = SharedMemoryGarbageCollector::createInstance();
	if (collector)
	{
		collector->run(Application::getUUID());
	}

	TaskManager::createScheduler(TabIds::app());
	TaskManager::createScheduler(TabIds::background());
	MessageQueue::getInstance();

	s_instance = std::shared_ptr<Application>(new Application(hasGui));

	s_instance->m_storageCache = std::make_shared<StorageCache>();

	if (hasGui)
	{
		s_instance->m_mainView = viewFactory->createMainView(s_instance->m_storageCache.get());
		s_instance->m_mainView->setup();
	}

	if (networkFactory != nullptr)
	{
		s_instance->m_ideCommunicationController = networkFactory->createIDECommunicationController(
			s_instance->m_storageCache.get());
		s_instance->m_ideCommunicationController->startListening();
	}

	s_instance->startMessagingAndScheduling();
}

std::shared_ptr<Application> Application::getInstance()
{
	return s_instance;
}

void Application::destroyInstance()
{
	MessageQueue::getInstance()->stopMessageLoop();
	TaskManager::destroyScheduler(TabIds::background());
	TaskManager::destroyScheduler(TabIds::app());

	s_instance.reset();
}

std::string Application::getUUID()
{
	if (!s_uuid.size())
	{
		s_uuid = utility::getUuidString();
	}

	return s_uuid;
}

void Application::loadSettings()
{
	MessageStatus("Load settings: " + UserPaths::getAppSettingsFilePath().str()).dispatch();

	std::shared_ptr<ApplicationSettings> settings = ApplicationSettings::getInstance();
	settings->load(UserPaths::getAppSettingsFilePath());

	LogManager::getInstance()->setLoggingEnabled(settings->getLoggingEnabled());
	Logger* logger = LogManager::getInstance()->getLoggerByType("FileLogger");
	if (logger)
	{
		auto *fileLogger = dynamic_cast<FileLogger*>(logger);
		fileLogger->setLogDirectory(settings->getLogDirectoryPath());
		fileLogger->setFileName(FileLogger::generateDatedFileName("log"));
	}

	loadStyle(settings->getColorSchemePath());
}

void Application::loadStyle(const FilePath& colorSchemePath)
{
	ColorScheme::getInstance()->load(colorSchemePath);
	GraphViewStyle::loadStyleSettings();
}

Application::Application(bool withGUI): m_hasGUI(withGUI) {}

Application::~Application()
{
	if (m_hasGUI)
	{
		m_mainView->saveLayout();
	}

	SharedMemoryGarbageCollector* collector = SharedMemoryGarbageCollector::getInstance();
	if (collector)
	{
		collector->stop();
	}
}

std::shared_ptr<const Project> Application::getCurrentProject() const
{
	return m_project;
}

FilePath Application::getCurrentProjectPath() const
{
	if (m_project)
	{
		return m_project->getProjectSettingsFilePath();
	}

	return FilePath();
}

bool Application::isProjectLoaded() const
{
	if (m_project)
	{
		return m_project->isLoaded();
	}
	return false;
}

bool Application::hasGUI()
{
	return m_hasGUI;
}

int Application::handleDialog(const std::string& message)
{
	return getDialogView(DialogView::UseCase::GENERAL)->confirm(message);
}

int Application::handleDialog(const std::string& message, const std::vector<std::string>& options)
{
	return getDialogView(DialogView::UseCase::GENERAL)->confirm(message, options);
}

std::shared_ptr<DialogView> Application::getDialogView(DialogView::UseCase useCase)
{
	if (m_mainView)
	{
		return m_mainView->getDialogView(useCase);
	}

	return std::make_shared<DialogView>(useCase, nullptr);
}

void Application::updateHistoryMenu(std::shared_ptr<MessageBase> message)
{
	m_mainView->updateHistoryMenu(message);
}

void Application::updateBookmarks(const std::vector<std::shared_ptr<Bookmark>>& bookmarks)
{
	m_mainView->updateBookmarksMenu(bookmarks);
}

void Application::handleMessage(MessageActivateWindow*  /*message*/)
{
	if (m_hasGUI)
	{
		m_mainView->activateWindow();
	}
}

void Application::handleMessage(MessageCloseProject*  /*message*/)
{
	if (m_project && m_project->isIndexing())
	{
		MessageStatus("Cannot close the project while indexing.", true, false).dispatch();
		return;
	}

	m_project.reset();
	updateTitle();
	m_mainView->clear();
}

void Application::handleMessage(MessageIndexingFinished*  /*message*/)
{
	logStorageStats();

	if (m_hasGUI)
	{
		MessageRefreshUI().afterIndexing().dispatch();
	}
	else
	{
		MessageQuitApplication().dispatch();
	}
}

void Application::handleMessage(MessageLoadProject* message)
{
	TRACE("app load project");

	FilePath projectSettingsFilePath(message->projectSettingsFilePath);
	loadWindow(projectSettingsFilePath.empty());

	if (projectSettingsFilePath.empty())
	{
		return;
	}

	if (m_project && m_project->isIndexing())
	{
		MessageStatus("Cannot load another project while indexing.", true, false).dispatch();
		return;
	}

	if (m_project && projectSettingsFilePath == m_project->getProjectSettingsFilePath())
	{
		if (message->settingsChanged && m_hasGUI)
		{
			m_project->setStateOutdated();
			refreshProject(RefreshMode::ALL_FILES, message->shallowIndexingRequested);
		}
	}
	else
	{
		MessageStatus("Loading Project: " + projectSettingsFilePath.str(), false, true).dispatch();

		m_project.reset();

		if (m_hasGUI)
		{
			m_mainView->clear();
		}

		try
		{
			updateRecentProjects(projectSettingsFilePath);

			m_storageCache->clear();
			m_storageCache->setSubject(
				std::weak_ptr<StorageAccess>());	// TODO: check if this is really required.

			m_project = std::make_shared<Project>(
				std::make_shared<ProjectSettings>(projectSettingsFilePath),
				m_storageCache.get(),
				getUUID(),
				hasGUI());

			if (m_project)
			{
				m_project->load(getDialogView(DialogView::UseCase::GENERAL));
			}
			else
			{
				LOG_ERROR_STREAM(<< "Failed to load project.");
				MessageStatus("Failed to load project: " + projectSettingsFilePath.str(), true)
					.dispatch();
			}

			updateTitle();
		}
		catch (CppSQLite3Exception& e)
		{
			const std::string message = "Failed to load project at \"" +
										 projectSettingsFilePath.str() + "\" with sqlite exception: " +
										 e.errorMessage();
			LOG_ERROR(message);
			MessageStatus(message, true).dispatch();
		}
		catch (std::exception& e)
		{
			const std::string message = "Failed to load project at \"" +
				projectSettingsFilePath.str() + "\" with exception: " +
				e.what();
			LOG_ERROR(message);
			MessageStatus(message, true).dispatch();
		}
		catch (...)
		{
			const std::string message = "Failed to load project at \"" +
				projectSettingsFilePath.str() + "\" with unknown exception.";
			LOG_ERROR(message);
			MessageStatus(message, true).dispatch();
		}

		if (message->refreshMode != RefreshMode::NONE)
		{
			refreshProject(message->refreshMode, message->shallowIndexingRequested);
		}
	}
}

void Application::handleMessage(MessageRefresh* message)
{
	TRACE("app refresh");

	refreshProject(message->all ? RefreshMode::ALL_FILES : RefreshMode::UPDATED_FILES, false);
}

void Application::handleMessage(MessageRefreshUI* message)
{
	TRACE("ui refresh");

	if (m_hasGUI)
	{
		updateTitle();

		if (message->loadStyle)
		{
			loadStyle(ApplicationSettings::getInstance()->getColorSchemePath());
		}

		m_mainView->refreshViews();

		m_mainView->refreshUIState(message->isAfterIndexing);
	}
}

void Application::handleMessage(MessageSwitchColorScheme* message)
{
	MessageStatus("Switch color scheme: " + message->colorSchemePath.str()).dispatch();

	loadStyle(message->colorSchemePath);
	MessageRefreshUI().noStyleReload().dispatch();
}

void Application::startMessagingAndScheduling()
{
	TaskManager::getScheduler(TabIds::app())->startSchedulerLoopThreaded();
	TaskManager::getScheduler(TabIds::background())->startSchedulerLoopThreaded();

	MessageQueue* queue = MessageQueue::getInstance().get();
	queue->addMessageFilter(std::make_shared<MessageFilterErrorCountUpdate>());
	queue->addMessageFilter(std::make_shared<MessageFilterFocusInOut>());
	queue->addMessageFilter(std::make_shared<MessageFilterSearchAutocomplete>());

	queue->setSendMessagesAsTasks(true);
	queue->startMessageLoopThreaded();
}

void Application::loadWindow(bool showStartWindow)
{
	if (!m_hasGUI)
	{
		return;
	}

	if (!m_loadedWindow)
	{
		auto appSettings = ApplicationSettings::getInstance();

		updateTitle();

		m_mainView->loadWindow(showStartWindow);
		m_loadedWindow = true;
	}
	else if (!showStartWindow)
	{
		m_mainView->hideStartScreen();
	}
}

void Application::refreshProject(RefreshMode refreshMode, bool shallowIndexingRequested)
{
	if (m_project && checkSharedMemory())
	{
		m_project->refresh(
			getDialogView(DialogView::UseCase::INDEXING), refreshMode, shallowIndexingRequested);

		if (!m_hasGUI && !m_project->isIndexing())
		{
			MessageQuitApplication().dispatch();
		}
	}
}

void Application::updateRecentProjects(const FilePath& projectSettingsFilePath)
{
	if (m_hasGUI)
	{
		ApplicationSettings* appSettings = ApplicationSettings::getInstance().get();
		std::vector<FilePath> recentProjects = appSettings->getRecentProjects();
		if (recentProjects.size())
		{
			std::vector<FilePath>::iterator it = std::find(
				recentProjects.begin(), recentProjects.end(), projectSettingsFilePath);
			if (it != recentProjects.end())
			{
				recentProjects.erase(it);
			}
		}

		recentProjects.insert(recentProjects.begin(), projectSettingsFilePath);
		while (recentProjects.size() > ApplicationSettings::getMaxRecentProjectsCount())
		{
			recentProjects.pop_back();
		}

		appSettings->setRecentProjects(recentProjects);
		appSettings->save(UserPaths::getAppSettingsFilePath());

		m_mainView->updateRecentProjectMenu();
	}
}

void Application::logStorageStats() const
{
	if (!ApplicationSettings::getInstance()->getLoggingEnabled())
	{
		return;
	}

	std::stringstream ss;
	StorageStats stats = m_storageCache->getStorageStats();

	ss << "\nGraph:\n";
	ss << "\t" << stats.nodeCount << " Nodes\n";
	ss << "\t" << stats.edgeCount << " Edges\n";

	ss << "\nCode:\n";
	ss << "\t" << stats.fileCount << " Files\n";
	ss << "\t" << stats.fileLOCCount << " Lines of Code\n";


	ErrorCountInfo errorCount = m_storageCache->getErrorCount();

	ss << "\nErrors:\n";
	ss << "\t" << errorCount.total << " Errors\n";
	ss << "\t" << errorCount.fatal << " Fatal Errors\n";

	LOG_INFO(ss.str());
}

void Application::updateTitle()
{
	if (m_hasGUI)
	{
		std::string title = "Sourcetrail";

		if (m_project)
		{
			FilePath projectPath = m_project->getProjectSettingsFilePath();

			if (!projectPath.empty())
			{
				title += " - " + projectPath.fileName();
			}
		}

		m_mainView->setTitle(title);
	}
}

bool Application::checkSharedMemory()
{
	std::string error = SharedMemory::checkSharedMemory(getUUID());
	if (error.size())
	{
		MessageStatus(
			"Error on accessing shared memory. Indexing not possible. "
			"Please restart computer or run as admin: " +
				error,
			true)
			.dispatch();
		handleDialog(
			"There was an error accessing shared memory on your computer: " + error +
			"\n\n"
			"Project indexing is not possible. Please restart your computer or try running "
			"Sourcetrail as admin.");
		return false;
	}

	return true;
}
