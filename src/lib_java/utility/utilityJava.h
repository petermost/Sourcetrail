#ifndef UTILITY_JAVA_H
#define UTILITY_JAVA_H

#include <set>
#include <string>
#include <vector>

class FilePath;

namespace utility
{
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
