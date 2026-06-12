#include "setupApp.h"

#include "Application.h"
#include "ApplicationSettings.h"
#include "ApplicationSettingsPrefiller.h"
#include "CommandLineParser.h"
#include "ConsoleLogger.h"
#include "FileLogger.h"
#include "LanguagePackageManager.h"
#include "LogManager.h"
#include "MessageIndexingInterrupted.h"
#include "MessageLoadProject.h"
#include "MessageStatus.h"
#include "Platform.h"
#include "QtApplication.h"
#include "QtCoreApplication.h"
#include "QtNetworkFactory.h"
#include "QtViewFactory.h"
#include "ResourcePaths.h"
#include "ScopedFunctor.h"
#include "SourceGroupFactory.h"
#include "SourceGroupFactoryModuleCustom.h"
#include "language_packages.h"
#include "utilityQt.h"
#include "Version.h"
#include "utilityString.h"

#if BUILD_CXX_LANGUAGE_PACKAGE
	#include "LanguagePackageCxx.h"
	#include "SourceGroupFactoryModuleCxx.h"
#endif	  // BUILD_CXX_LANGUAGE_PACKAGE

#if BUILD_JAVA_LANGUAGE_PACKAGE
	#include "LanguagePackageJava.h"
	#include "SourceGroupFactoryModuleJava.h"
#endif	  // BUILD_JAVA_LANGUAGE_PACKAGE

#if BOOST_OS_WINDOWS
	#include <windows.h>
#endif

#include <QByteArray>
#include <QtEnvironmentVariables>

#include <csignal>
#include <iostream>

using namespace utility;
using namespace std;
using namespace boost::filesystem;

// Sourcetrail is now a GUI application i.e.: add_executable(Sourcetrail WIN32 ...), which means that Windows will not open a console window.
// See issue "The console window is not hidden under Windows 11" (https://github.com/petermost/Sourcetrail/issues/19)
// To still get log messages in the console, we try to attach to an existing console.
// This means Sourcetrail will have the same behaviour under Linux and Windows:
// - Start from file manager: No console with log messages
// - Start from console and logging disabled: No log messages
// - Start from console and logging enabled: Log messages
// Summary: To get console log messages, Sourcetrail must be started from within a console and logging must be enabled.

void setupConsoleWindow()
{
#if BOOST_OS_WINDOWS
	bool hasConsole = false;

	// Try to attach to the existing parent console:
	if (AttachConsole(ATTACH_PARENT_PROCESS))
	{
		hasConsole = true;
	}
#if 0 // Disabled for now, until we know that the new behaviour is satisfactory.
	// Otherwise create/open a new console:
	else if (AllocConsole())
	{
		// The newly opened console window will gain the focus and will be in front of the main window,
		// but this seems acceptable for users who have explicitly enabled/requested console logging.

		hasConsole = true;
		SetConsoleTitleW(L"Sourcetrail Console Logging");
	}
#endif
	if (hasConsole)
	{
		// Redirect C streams:

		FILE* fpDummy;
		freopen_s(&fpDummy, "CONOUT$", "w", stdout);
		freopen_s(&fpDummy, "CONOUT$", "w", stderr);
		freopen_s(&fpDummy, "CONIN$", "r", stdin);

		// Redirect Windows handles:

		HANDLE hOut = CreateFileW(L"CONOUT$", GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
		HANDLE hIn = CreateFileW(L"CONIN$", GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);

		SetStdHandle(STD_OUTPUT_HANDLE, hOut);
		SetStdHandle(STD_ERROR_HANDLE, hOut);
		SetStdHandle(STD_INPUT_HANDLE, hIn);

		// Clear error flags and sync C++ streams:

		std::clog.clear();
		std::cerr.clear();
		std::cout.clear();
		std::cin.clear();

		std::ios::sync_with_stdio(true);
	}
#endif
}

void signalHandler(int  /*signum*/)
{
	std::cout << "interrupt indexing" << std::endl;
	MessageIndexingInterrupted().dispatch();
}

static void setupLogging(const ApplicationSettings *settings)
{
	std::shared_ptr<LogManager> logManager = LogManager::getInstance();
	logManager->setLoggingEnabled(settings->getLoggingEnabled());

	std::shared_ptr<ConsoleLogger> consoleLogger = std::make_shared<ConsoleLogger>();
	consoleLogger->setLogLevel(Logger::LOG_ALL);
	logManager->addLogger(consoleLogger);

	std::shared_ptr<FileLogger> fileLogger = std::make_shared<FileLogger>();
	fileLogger->setLogLevel(Logger::LOG_ALL);
	fileLogger->setLogDirectory(settings->getLogDirectoryPath());
	fileLogger->setFileName(FileLogger::generateDatedFileName("log"));
	logManager->addLogger(fileLogger);

	fileLogger->deleteLogFiles(FileLogger::generateDatedFileName("log", -30));
}

void addLanguagePackages()
{
	SourceGroupFactory::getInstance()->addModule(std::make_shared<SourceGroupFactoryModuleCustom>());

#if BUILD_CXX_LANGUAGE_PACKAGE
	SourceGroupFactory::getInstance()->addModule(std::make_shared<SourceGroupFactoryModuleCxx>());
#endif

#if BUILD_JAVA_LANGUAGE_PACKAGE
	SourceGroupFactory::getInstance()->addModule(std::make_shared<SourceGroupFactoryModuleJava>());
#endif

#if BUILD_CXX_LANGUAGE_PACKAGE
	LanguagePackageManager::getInstance()->addPackage(std::make_shared<LanguagePackageCxx>());
#endif

#if BUILD_JAVA_LANGUAGE_PACKAGE
	LanguagePackageManager::getInstance()->addPackage(std::make_shared<LanguagePackageJava>());
#endif
}

int main(int argc, char* argv[])
{
	setupDefaultLocale();

	// Must get the correct directory for:
	// Windows: 'Sourcetrail' (doesn't exist, so canonical() would fail!)
	// Windows: 'Sourcetrail.exe'
	// Linux:   './Sourcetrail'

	const path appDirectory = weakly_canonical(argv[0]).parent_path();
	setupAppDirectories(appDirectory.generic_string());

	if constexpr (utility::Platform::isLinux())
	{
		if (qgetenv("SOURCETRAIL_VIA_SCRIPT").isNull())
		{
			std::cout << "ERROR: Please run Sourcetrail via the Sourcetrail.sh script!" << std::endl;
		}
	}
	const Version version = Version::getApplicationVersion();
	MessageStatus("Starting Sourcetrail version "s + version.toDisplayString()).dispatch();
	MessageStatus("Setting application directory: "s + appDirectory.generic_string()).dispatch();

	commandline::CommandLineParser commandLineParser(version.toDisplayString());
	commandLineParser.preparse(argc, argv);
	if (commandLineParser.exitApplication())
	{
		return 0;
	}

	setupAppEnvironment(argc, argv);

	if (commandLineParser.runWithoutGUI())
	{
		// headless Sourcetrail
		[[maybe_unused]]
		QtCoreApplication qtApp(argc, argv);

		Application::createInstance(nullptr, nullptr);
		shared_ptr<ApplicationSettings> appSettings = ApplicationSettings::getInstance();

		setupLogging(appSettings.get());
		
		[[maybe_unused]]
		ScopedFunctor f([]()
		{
			Application::destroyInstance();
		});

		ApplicationSettingsPrefiller::prefillPaths(appSettings.get());
		addLanguagePackages();

		signal(SIGINT, signalHandler);
		signal(SIGTERM, signalHandler);
		signal(SIGABRT, signalHandler);

		commandLineParser.parse();

		if (commandLineParser.exitApplication())
		{
			return 0;
		}

		if (commandLineParser.hasError())
		{
			std::cout << commandLineParser.getError() << std::endl;
		}
		else
		{
			MessageLoadProject(commandLineParser.getProjectFilePath(), false, commandLineParser.getRefreshMode()).dispatch();
		}

		return QtCoreApplication::exec();
	}
	else
	{
		[[maybe_unused]]
		QtApplication qtApp(argc, argv);

		QtViewFactory viewFactory;
		QtNetworkFactory networkFactory;

		Application::createInstance(&viewFactory, &networkFactory);
		shared_ptr<ApplicationSettings> appSettings = ApplicationSettings::getInstance();

		if (appSettings->getLoggingEnabled())
			setupConsoleWindow();

		setupLogging(appSettings.get());

		[[maybe_unused]]
		ScopedFunctor f([]()
		{
			Application::destroyInstance();
		});

		ApplicationSettingsPrefiller::prefillPaths(appSettings.get());
		addLanguagePackages();

		utility::loadFontsFromDirectory(ResourcePaths::getFontsDirectoryPath(), ".otf");
		utility::loadFontsFromDirectory(ResourcePaths::getFontsDirectoryPath(), ".ttf");

		if (commandLineParser.hasError())
		{
			Application::getInstance()->handleDialog(commandLineParser.getError());
		}
		else
		{
			MessageLoadProject(commandLineParser.getProjectFilePath(), false, RefreshMode::NONE).dispatch();
		}

		return QtApplication::exec();
	}
}
