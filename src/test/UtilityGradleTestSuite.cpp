#include "Catch2.hpp"

#include "language_packages.h"

#if BUILD_JAVA_LANGUAGE_PACKAGE

#include "FilePath.h"
#include "FileSystem.h"
#include "utility.h"
#include "utilityGradle.h"

#include <utilityApp.h>

using namespace utility;

namespace
{

FilePath tmpFolder = FilePath("data/UtilityGradleTestSuite/tmp");

void cleanup()
{
	if (tmpFolder.recheckExists())
	{
		for (const FilePath& path: FileSystem::getFilePathsFromDirectory(tmpFolder))
		{
			FileSystem::remove(path);
		}
		FileSystem::remove(tmpFolder);
	}
}

}

TEST_CASE("gradle wrapper detects source directories of simple projects", JAVA_TAG)
{
	std::vector<FilePath> result = utility::gradleGetAllSourceDirectories(
		FilePath("data/UtilityGradleTestSuite/simple_gradle_project"), false);

	REQUIRE(utility::containsElement<FilePath>(
		result,
		FilePath("data/UtilityGradleTestSuite/simple_gradle_project/src/main/java").makeAbsolute()));

	REQUIRE(!utility::containsElement<FilePath>(
		result,
		FilePath("data/UtilityGradleTestSuite/simple_gradle_project/src/test/java").makeAbsolute()));
}

TEST_CASE("gradle wrapper detects source and test directories of simple projects", JAVA_TAG)
{
	std::vector<FilePath> result = utility::gradleGetAllSourceDirectories(
		FilePath("data/UtilityGradleTestSuite/simple_gradle_project"), true);

	REQUIRE(utility::containsElement<FilePath>(
		result,
		FilePath("data/UtilityGradleTestSuite/simple_gradle_project/src/main/java").makeAbsolute()));

	REQUIRE(utility::containsElement<FilePath>(
		result,
		FilePath("data/UtilityGradleTestSuite/simple_gradle_project/src/test/java").makeAbsolute()));
}

TEST_CASE("gradle wrapper detects source dependencies of simple projects", JAVA_TAG)
{
	std::vector<std::string> requiredDependencies = {"joda-time-2.2.jar"};

	cleanup();
	REQUIRE(utility::gradleCopyDependencies(
		FilePath("data/UtilityGradleTestSuite/simple_gradle_project"),
		tmpFolder.makeAbsolute(),
		false));

	const std::vector<FilePath> copiedDependencies = FileSystem::getFilePathsFromDirectory(tmpFolder);

	for (const std::string &requiredDependency: requiredDependencies)
	{
		REQUIRE(utility::containsElement(
			copiedDependencies, tmpFolder.getConcatenated(requiredDependency)));
	}
	cleanup();
}

TEST_CASE("gradle wrapper detects source and test dependencies of simple projects", JAVA_TAG)
{
	std::vector<std::string> requiredDependencies = {
		"joda-time-2.2.jar", "hamcrest-core-1.3.jar", "junit-4.12.jar"};

	cleanup();
	REQUIRE(utility::gradleCopyDependencies(
		FilePath("data/UtilityGradleTestSuite/simple_gradle_project"),
		tmpFolder.makeAbsolute(),
		true));

	const std::vector<FilePath> copiedDependencies = FileSystem::getFilePathsFromDirectory(tmpFolder);

	for (const std::string &requiredDependency: requiredDependencies)
	{
		REQUIRE(utility::containsElement(
			copiedDependencies, tmpFolder.getConcatenated(requiredDependency)));
	}
	cleanup();
}

#endif
