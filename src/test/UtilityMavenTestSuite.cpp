#include "Catch2.hpp"

#include "language_packages.h"

#if BUILD_JAVA_LANGUAGE_PACKAGE

#	include "FilePath.h"
#	include "utility.h"
#	include "utilityMaven.h"
#	include "utilityPathDetection.h"

static std::shared_ptr<PathDetector> getMavenPathDetector()
{
	std::shared_ptr<PathDetector> mavenPathDetector = utility::getMavenExecutablePathDetector();
	REQUIRE_MESSAGE("Maven command not found", mavenPathDetector->isWorking());

	return mavenPathDetector;
}

TEST_CASE("maven path detector is working")
{
	REQUIRE(getMavenPathDetector() != nullptr);
}

TEST_CASE("maven wrapper detects source directories of simple projects")
{
	std::shared_ptr<PathDetector> mavenPathDetector = getMavenPathDetector();

	if (!mavenPathDetector->getPaths().empty())
	{
		std::vector<FilePath> result = utility::mavenGetAllDirectoriesFromEffectivePom(
			mavenPathDetector->getPaths().front(),
			FilePath(),
			FilePath("data/UtilityMavenTestSuite/simple_maven_project"),
			FilePath("data/UtilityMavenTestSuite").makeAbsolute(),
			false);

		REQUIRE(utility::containsElement<FilePath>(
			result,
			FilePath("data/UtilityMavenTestSuite/simple_maven_project/src/main/java").makeAbsolute()));

		REQUIRE(!utility::containsElement<FilePath>(
			result,
			FilePath("data/UtilityMavenTestSuite/simple_maven_project/src/test/java").makeAbsolute()));
	}
}

TEST_CASE("maven wrapper detects source and test directories of simple projects")
{
	std::shared_ptr<PathDetector> mavenPathDetector = getMavenPathDetector();

	if (!mavenPathDetector->getPaths().empty())
	{
		std::vector<FilePath> result = utility::mavenGetAllDirectoriesFromEffectivePom(
			mavenPathDetector->getPaths().front(),
			FilePath(),
			FilePath("data/UtilityMavenTestSuite/simple_maven_project"),
			FilePath("data/UtilityMavenTestSuite").makeAbsolute(),
			true);

		REQUIRE(utility::containsElement<FilePath>(
			result,
			FilePath("data/UtilityMavenTestSuite/simple_maven_project/src/main/java").makeAbsolute()));

		REQUIRE(utility::containsElement<FilePath>(
			result,
			FilePath("data/UtilityMavenTestSuite/simple_maven_project/src/test/java").makeAbsolute()));
	}
}

TEST_CASE("maven wrapper detects source directories of nested modules")
{
	std::shared_ptr<PathDetector> mavenPathDetector = getMavenPathDetector();

	if (!mavenPathDetector->getPaths().empty())
	{
		std::vector<FilePath> result = utility::mavenGetAllDirectoriesFromEffectivePom(
			mavenPathDetector->getPaths().front(),
			FilePath(),
			FilePath("data/UtilityMavenTestSuite/nested_maven_project"),
			FilePath("data/UtilityMavenTestSuite").makeAbsolute(),
			false);

		REQUIRE(utility::containsElement<FilePath>(
			result,
			FilePath("data/UtilityMavenTestSuite/nested_maven_project/module_1/src/main/java")
				.makeAbsolute()));

		REQUIRE(utility::containsElement<FilePath>(
			result,
			FilePath("data/UtilityMavenTestSuite/nested_maven_project/module_2/src/main/java")
				.makeAbsolute()));

		REQUIRE(!utility::containsElement<FilePath>(
			result,
			FilePath("data/UtilityMavenTestSuite/nested_maven_project/module_1/src/test/java")
				.makeAbsolute()));

		REQUIRE(!utility::containsElement<FilePath>(
			result,
			FilePath("data/UtilityMavenTestSuite/nested_maven_project/module_2/src/test/java")
				.makeAbsolute()));
	}
}

TEST_CASE("maven wrapper detects source and test directories of nested modules")
{
	std::shared_ptr<PathDetector> mavenPathDetector = getMavenPathDetector();

	if (!mavenPathDetector->getPaths().empty())
	{
		std::vector<FilePath> result = utility::mavenGetAllDirectoriesFromEffectivePom(
			mavenPathDetector->getPaths().front(),
			FilePath(),
			FilePath("data/UtilityMavenTestSuite/nested_maven_project"),
			FilePath("data/UtilityMavenTestSuite").makeAbsolute(),
			true);

		REQUIRE(utility::containsElement<FilePath>(
			result,
			FilePath("data/UtilityMavenTestSuite/nested_maven_project/module_1/src/main/java")
				.makeAbsolute()));

		REQUIRE(utility::containsElement<FilePath>(
			result,
			FilePath("data/UtilityMavenTestSuite/nested_maven_project/module_2/src/main/java")
				.makeAbsolute()));

		REQUIRE(utility::containsElement<FilePath>(
			result,
			FilePath("data/UtilityMavenTestSuite/nested_maven_project/module_1/src/test/java")
				.makeAbsolute()));

		REQUIRE(utility::containsElement<FilePath>(
			result,
			FilePath("data/UtilityMavenTestSuite/nested_maven_project/module_2/src/test/java")
				.makeAbsolute()));
	}
}

#endif	  // BUILD_JAVA_LANGUAGE_PACKAGE
