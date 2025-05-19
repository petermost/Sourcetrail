#include "Catch2.hpp"

#include "ApplicationSettings.h"
#include "language_packages.h"
#include "utilityPathDetection.h"
#include <AppPath.h>
#include <UserPaths.h>
#include <setupApp.h>

#include <boost/filesystem.hpp>

#include <iostream>

using namespace std;
using namespace boost::filesystem;
using namespace utility;

struct EventListener : Catch2::EventListenerBase
{
	static int s_argc;
	static char **s_argv;

	using Catch2::EventListenerBase::EventListenerBase;	   // inherit constructor

	void testRunStarting(const Catch::TestRunInfo& ) override
	{
		FilePath appPath = FilePath(s_argv[0]).getCanonical().getParentDirectory().getParentDirectory().getConcatenated("app");
		cout << "Setting 'app' directory to " << appPath.str() << endl;
		setupAppDirectories(appPath);

		FilePath settingsFilePath = UserPaths::getAppSettingsFilePath();
		cout << "Loading settings from " << settingsFilePath.str() << endl;
		ApplicationSettings::getInstance()->load(settingsFilePath, true);

#if BUILD_JAVA_LANGUAGE_PACKAGE
		if (ApplicationSettings::getInstance()->getJavaPath().empty())
		{
			vector<FilePath> javaPaths = utility::getJavaRuntimePathDetector()->getPaths();
			if (!javaPaths.empty())
			{
				ApplicationSettings::getInstance()->setJavaPath(javaPaths.front());
				cout << "Java path written to settings: " << ApplicationSettings::getInstance()->getJavaPath().str() << endl;
			}
			else
				cout << "Java path not found in settings or PATH/JAVA_HOME environment variable!" << endl;
		}
		else
			cout << "Java path read from settings: " << ApplicationSettings::getInstance()->getJavaPath().str() << endl;
#endif
	}
};

int EventListener::s_argc = 0;
char **EventListener::s_argv = nullptr;

CATCH_REGISTER_LISTENER(EventListener)

int main(int argc, char* argv[])
{
	EventListener::s_argc = argc;
	EventListener::s_argv = argv;

	setupDefaultLocale();

	// Set the 'working directory' manually, as a workaround for "Unable to configure working directory
	// in CMake/Catch" (https://github.com/catchorg/Catch2/issues/2249)
	path workingDirectory = canonical(path(argv[0])).parent_path();

	// If something is printed to the screen, then this will lead to a failure in 'catch_discover_tests()'!
	// cout << "Set working directory to '" << workingDirectory << "'" << endl;
	current_path(workingDirectory);

	return Catch::Session().run( argc, argv );
}
