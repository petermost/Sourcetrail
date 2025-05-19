#include "utilityGradle.h"

#include <set>

#include "Application.h"
#include "JavaEnvironment.h"
#include "JavaEnvironmentFactory.h"
#include "MessageStatus.h"
#include "ResourcePaths.h"
#include "logging.h"
#include "utilityJava.h"
#include "utilityString.h"

namespace utility
{
bool gradleCopyDependencies(
	const FilePath& projectDirectoryPath, const FilePath& outputDirectoryPath, bool addTestDependencies)
{
	const FilePath gradleInitScriptPath = ResourcePaths::getJavaDirectoryPath().concatenate(
		"gradle/init.gradle");

	utility::setJavaHomeVariableIfNotExists();
	utility::prepareJavaEnvironment();

	std::shared_ptr<JavaEnvironment> javaEnvironment =
		JavaEnvironmentFactory::getInstance()->createEnvironment();
	bool success = javaEnvironment->callStaticVoidMethod(
		"com/sourcetrail/gradle/InfoRetriever",
		"copyCompileLibs",
		utility::encodeToUtf8(projectDirectoryPath.str()),
		utility::encodeToUtf8(gradleInitScriptPath.str()),
		utility::encodeToUtf8(outputDirectoryPath.str()));

	if (success && addTestDependencies)
	{
		success = javaEnvironment->callStaticVoidMethod(
			"com/sourcetrail/gradle/InfoRetriever",
			"copyTestCompileLibs",
			utility::encodeToUtf8(projectDirectoryPath.str()),
			utility::encodeToUtf8(gradleInitScriptPath.str()),
			utility::encodeToUtf8(outputDirectoryPath.str()));
	}

	return success;
}

std::vector<FilePath> gradleGetAllSourceDirectories(
	const FilePath& projectDirectoryPath, bool addTestDirectories)
{
	const FilePath gradleInitScriptPath = ResourcePaths::getJavaDirectoryPath().concatenate(
		"gradle/init.gradle");

	utility::setJavaHomeVariableIfNotExists();
	utility::prepareJavaEnvironment();

	std::set<std::string> uncheckedDirectories;
	{
		std::shared_ptr<JavaEnvironment> javaEnvironment =
			JavaEnvironmentFactory::getInstance()->createEnvironment();
		{
			std::string output;
			javaEnvironment->callStaticStringMethod(
				"com/sourcetrail/gradle/InfoRetriever",
				"getMainSrcDirs",
				output,
				utility::encodeToUtf8(projectDirectoryPath.str()),
				utility::encodeToUtf8(gradleInitScriptPath.str()));

			if (utility::isPrefix("[ERROR]", utility::trim(output)))
			{
				// TODO: move error handling to caller of this function
				const std::string dialogMessage =
					"The following error occurred while executing a Gradle command:\n\n" +
					utility::decodeFromUtf8(utility::replace(output, "\r\n", "\n"));
				MessageStatus(dialogMessage, true, false).dispatch();
				if (Application::getInstance())
				{
					Application::getInstance()->handleDialog(dialogMessage);
				}
				return std::vector<FilePath>();
			}

			for (const std::string &mainSrcDir: utility::splitToVector(output, ";"))
			{
				uncheckedDirectories.insert(utility::decodeFromUtf8(mainSrcDir));
			}
		}

		if (addTestDirectories)
		{
			std::string output;
			javaEnvironment->callStaticStringMethod(
				"com/sourcetrail/gradle/InfoRetriever",
				"getTestSrcDirs",
				output,
				utility::encodeToUtf8(projectDirectoryPath.str()),
				utility::encodeToUtf8(gradleInitScriptPath.str()));

			if (utility::isPrefix("[ERROR]", utility::trim(output)))
			{
				// TODO: move error handling to caller of this function
				const std::string dialogMessage =
					"The following error occurred while executing a Gradle command:\n\n" +
					utility::decodeFromUtf8(utility::replace(output, "\r\n", "\n"));
				MessageStatus(dialogMessage, true, false).dispatch();
				Application::getInstance()->handleDialog(dialogMessage);

				return std::vector<FilePath>();
			}

			for (const std::string &testSrcDir: utility::splitToVector(output, ";"))
			{
				uncheckedDirectories.insert(utility::decodeFromUtf8(testSrcDir));
			}
		}
	}

	std::vector<FilePath> directories;
	for (const std::string& uncheckedDirectory: uncheckedDirectories)
	{
		FilePath uncheckedDirectoryPath(uncheckedDirectory);
		if (uncheckedDirectoryPath.exists())
		{
			directories.push_back(uncheckedDirectoryPath);
		}
	}

	LOG_INFO(
		"Found " + std::to_string(directories.size()) + " of " +
		std::to_string(uncheckedDirectories.size()) + " directories on system.");

	return directories;
}
}	 // namespace utility
