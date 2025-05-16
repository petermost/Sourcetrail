#include "UserPaths.h"

FilePath UserPaths::s_userDataDirectoryPath;

FilePath UserPaths::getUserDataDirectoryPath()
{
	return s_userDataDirectoryPath;
}

void UserPaths::setUserDataDirectoryPath(const FilePath& path)
{
	s_userDataDirectoryPath = path;
}

FilePath UserPaths::getAppSettingsFilePath()
{
	return getUserDataDirectoryPath().concatenate("ApplicationSettings.xml");
}

FilePath UserPaths::getWindowSettingsFilePath()
{
	return getUserDataDirectoryPath().concatenate("window_settings.ini");
}

FilePath UserPaths::getLogDirectoryPath()
{
	return getUserDataDirectoryPath().concatenate("log/");
}
