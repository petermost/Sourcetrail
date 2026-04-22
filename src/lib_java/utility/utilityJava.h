#ifndef UTILITY_JAVA_H
#define UTILITY_JAVA_H

#include "Platform.h"

#include <set>
#include <string>
#include <vector>


class FilePath;

namespace utility
{

constexpr const char *getClassPathSeparator()
{
	if constexpr (utility::Platform::isWindows())
		return ";";
	else
		return ":";
}

std::vector<std::string> getRequiredJarNames();

std::string prepareJavaEnvironment();
std::string prepareJavaEnvironment(const FilePath &javaDirectoryPath);

bool prepareJavaEnvironmentAndDisplayOccurringErrors();

std::set<FilePath> fetchRootDirectories(const std::set<FilePath>& sourceFilePaths);

std::vector<FilePath> getClassPath(
	const std::vector<FilePath>& classpathItems,
	bool useJreSystemLibrary,
	const std::set<FilePath>& sourceFilePaths);

void setJavaHomeVariableIfNotExists();
}

#endif
