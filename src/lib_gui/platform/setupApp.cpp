#include "setupApp.h"

#include <AppPath.h>
#include <ApplicationSettings.h>
#include <FilePath.h>
#include <FileSystem.h>
#include <ResourcePaths.h>
#include <UserPaths.h>
#include <Version.h>
#include <productVersion.h>
#include <utilityApp.h>
#include <utilityQt.h>

#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QStandardPaths>

#include <boost/locale/generator.hpp>

using namespace std;
using namespace utility;
using namespace boost::locale;
using namespace boost::system;
using namespace boost::filesystem;

void setupDefaultLocale()
{
	locale defaultLocale = generator().generate("");

	locale::global(defaultLocale);
	cout.imbue(defaultLocale);
	cerr.imbue(defaultLocale);
}


static void setupUserDirectory(const FilePath &appPath)
{
	// Determine user directory:
	QString userDataLocation = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/";
	UserPaths::setUserDataDirectoryPath(FilePath(userDataLocation.toStdString()).makeAbsolute());

	// Create missing user directory and copy default configurations:
	if (!UserPaths::getUserDataDirectoryPath().exists()) {
		FileSystem::createDirectories(UserPaths::getUserDataDirectoryPath());

		// Copy a default application settings file:
		FileSystem::copyFile(ResourcePaths::getFallbackDirectoryPath().concatenate("ApplicationSettings.xml"),
			UserPaths::getAppSettingsFilePath());

		// Copy a default windows settings file:
		FileSystem::copyFile(ResourcePaths::getFallbackDirectoryPath().concatenate("window_settings.ini"),
			UserPaths::getWindowSettingsFilePath());

		// Copy the example/tutorial projects:
		FileSystem::copyDirectory(appPath.getConcatenated("user").concatenate("projects"),
			UserPaths::getUserDataDirectoryPath().concatenate("projects"));

		// Add u+w permissions because the source files may be marked read-only in some Linux distros:
		for (recursive_directory_iterator it(UserPaths::getUserDataDirectoryPath().getPath()); it != recursive_directory_iterator(); ++it) {
			perms currentPermissions = status(*it).permissions();
			permissions(*it, currentPermissions | perms::owner_write);
		}
	}
}

Version setupAppDirectories(const FilePath &appPath)
{
	QCoreApplication::setApplicationName(QStringLiteral("Sourcetrail"));

	Version version(PRODUCT_VERSION_MAJOR, PRODUCT_VERSION_MINOR, PRODUCT_VERSION_PATCH);
	QCoreApplication::setApplicationVersion(QString::fromStdString(version.toDisplayString()));

	// Note: This functions is called from main in 'main.cpp' *AND* the main in 'test_main.cpp'.
	// If the appPath is incorrect then most of the Java tests will fail because the
	// 'app/data/java/lib/java-indexer.jar' will not be found!
	AppPath::setSharedDataDirectoryPath(appPath);
	AppPath::setCxxIndexerDirectoryPath(appPath);

	setupUserDirectory(appPath);

	return version;
}

void setupAppEnvironment(int  /*argc*/, char*  /*argv*/[])
{
	// This function will be called after setupAppDirectories, so UserPaths::setUserDataDirectoryPath
	// has been initialized and UserPaths::getAppSettingsFilePath will return the correct path.

	// TODO (petermost): Check https://doc.qt.io/qt-6/highdpi.html#environment-variable-reference
	if constexpr(Platform::isLinux()) {
		// Set QT screen scaling factor
		ApplicationSettings appSettings;
		appSettings.load(UserPaths::getAppSettingsFilePath(), true);

		qputenv("QT_AUTO_SCREEN_SCALE_FACTOR_SOURCETRAIL", qgetenv("QT_AUTO_SCREEN_SCALE_FACTOR"));
		qputenv("QT_SCALE_FACTOR_SOURCETRAIL", qgetenv("QT_SCALE_FACTOR"));

		int autoScaling = appSettings.getScreenAutoScaling();
		if (autoScaling != -1)
		{
			QByteArray bytes;
			bytes.setNum(autoScaling);
			qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", bytes);
		}

		float scaleFactor = appSettings.getScreenScaleFactor();
		if (scaleFactor > 0.0)
		{
			QByteArray bytes;
			bytes.setNum(scaleFactor);
			qputenv("QT_SCALE_FACTOR", bytes);
		}
	}
}

