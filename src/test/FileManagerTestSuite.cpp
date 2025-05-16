#include "Catch2.hpp"

#include "FileManager.h"
#include "FilePath.h"
#include "FilePathFilter.h"
#include "FileSystem.h"
#include "utility.h"

using namespace std::string_literals;
using namespace utility;

TEST_CASE("file manager has added file paths after first fetch")
{
	std::vector<FilePath> sourcePaths;
	sourcePaths.push_back(FilePath("./data/FileManagerTestSuite/src/"));
	sourcePaths.push_back(FilePath("./data/FileManagerTestSuite/include/"));
	std::vector<FilePath> headerPaths;
	std::vector<FilePathFilter> excludeFilters;

	std::vector<FilePath> filePaths = FileSystem::getFilePathsFromDirectory(
		FilePath("./data/FileManagerTestSuite/src/"));
	REQUIRE(filePaths.size() == 3);

	std::vector<std::string> sourceExtensions;
	for (const FilePath &p : filePaths) {
		sourceExtensions.push_back(p.extension());
	}
	REQUIRE(sourceExtensions.size() == 3);

	FileManager fm;
	fm.update(sourcePaths, excludeFilters, sourceExtensions);
	std::vector<FilePath> foundSourcePaths = utility::toVector(fm.getAllSourceFilePaths());

	REQUIRE(foundSourcePaths.size() == 3);
	REQUIRE(containsElement(foundSourcePaths, filePaths[0]));
	REQUIRE(containsElement(foundSourcePaths, filePaths[1]));
	REQUIRE(containsElement(foundSourcePaths, filePaths[2]));
}
