#include "Catch2.hpp"

#include <string>
#include <vector>

#include "FileSystem.h"
#include "utility.h"
#include "Platform.h"

namespace
{

/*
bool isInFiles(const std::set<FilePath>& files, const FilePath& filename)
{
	return std::end(files) != files.find(filename);
}
*/

bool isInFileInfos(const std::vector<FileInfo>& infos, const std::string& filename)
{
	for (const FileInfo& info: infos)
	{
		if (info.path.getAbsolute().str() == FilePath(filename).getCanonical().str())
		{
			return true;
		}
	}

	return false;
}

bool isInFileInfos(
	const std::vector<FileInfo>& infos, const std::string& filename, const std::string& filename2)
{
	for (const FileInfo& info: infos)
	{
		if (info.path.str() == FilePath(filename).getCanonical().str() ||
			info.path.str() == FilePath(filename2).getCanonical().str())
		{
			return true;
		}
	}

	return false;
}

}

TEST_CASE("find cpp files")
{
	std::vector<std::string> cppFiles = utility::convert<FilePath, std::string>(
		FileSystem::getFilePathsFromDirectory(FilePath("data/FileSystemTestSuite"), {".cpp"}),
		[](const FilePath& filePath) { return filePath.str(); });

	REQUIRE(cppFiles.size() == 4);
	REQUIRE(utility::containsElement<std::string>(cppFiles, "data/FileSystemTestSuite/main.cpp"));
	REQUIRE(utility::containsElement<std::string>(
		cppFiles, "data/FileSystemTestSuite/Settings/sample.cpp"));
	REQUIRE(
		utility::containsElement<std::string>(cppFiles, "data/FileSystemTestSuite/src/main.cpp"));
	REQUIRE(
		utility::containsElement<std::string>(cppFiles, "data/FileSystemTestSuite/src/test.cpp"));
}

TEST_CASE("find h files")
{
	std::vector<std::string> headerFiles = utility::convert<FilePath, std::string>(
		FileSystem::getFilePathsFromDirectory(FilePath("data/FileSystemTestSuite"), {".h"}),
		[](const FilePath& filePath) { return filePath.str(); });

	REQUIRE(headerFiles.size() == 3);
	REQUIRE(utility::containsElement<std::string>(
		headerFiles, "data/FileSystemTestSuite/tictactoe.h"));
	REQUIRE(utility::containsElement<std::string>(
		headerFiles, "data/FileSystemTestSuite/Settings/player.h"));
	REQUIRE(utility::containsElement<std::string>(
		headerFiles, "data/FileSystemTestSuite/src/test.h"));
}

TEST_CASE("find all source files")
{
	std::vector<std::string> sourceFiles = utility::convert<FilePath, std::string>(
		FileSystem::getFilePathsFromDirectory(
			FilePath("data/FileSystemTestSuite"), {".h", ".hpp", ".cpp"}),
		[](const FilePath& filePath) { return filePath.str(); });

	REQUIRE(sourceFiles.size() == 8);
}

TEST_CASE("find file infos ignore symlinks")
{
	ASSERT_SYMLINK_PLATFORM();

	std::vector<FilePath> directoryPaths;
	directoryPaths.push_back(FilePath("./data/FileSystemTestSuite/src"));

	std::vector<FileInfo> files = FileSystem::getFileInfosFromPaths(
		directoryPaths, {".h", ".hpp", ".cpp"}, false);

	REQUIRE(files.size() == 2);
	REQUIRE(isInFileInfos(files, "./data/FileSystemTestSuite/src/test.cpp"));
	REQUIRE(isInFileInfos(files, "./data/FileSystemTestSuite/src/test.h"));
}

TEST_CASE("find file infos follow symlinks")
{
	ASSERT_SYMLINK_PLATFORM();

	std::vector<FilePath> directoryPaths;
	directoryPaths.push_back(FilePath("./data/FileSystemTestSuite/src"));

	std::vector<FileInfo> files = FileSystem::getFileInfosFromPaths(
		directoryPaths, {".h", ".hpp", ".cpp"}, true);

	REQUIRE(files.size() == 5);
	REQUIRE(isInFileInfos(
		files,
		"./data/FileSystemTestSuite/src/Settings/player.h",
		"./data/FileSystemTestSuite/player.h"));
	REQUIRE(isInFileInfos(
		files,
		"./data/FileSystemTestSuite/src/Settings/sample.cpp",
		"./data/FileSystemTestSuite/sample.cpp"));
	REQUIRE(isInFileInfos(
		files,
		"./data/FileSystemTestSuite/src/main.cpp",
		"./data/FileSystemTestSuite/src/Settings/src/main.cpp"));
	REQUIRE(isInFileInfos(
		files,
		"./data/FileSystemTestSuite/src/test.cpp",
		"./data/FileSystemTestSuite/src/Settings/src/test.cpp"));
	REQUIRE(isInFileInfos(
		files,
		"./data/FileSystemTestSuite/src/test.h",
		"./data/FileSystemTestSuite/src/Settings/src/test.h"));
}

TEST_CASE("find symlinked directories")
{
	ASSERT_SYMLINK_PLATFORM();

	std::vector<FilePath> directoryPaths;
	directoryPaths.push_back(FilePath("./data/FileSystemTestSuite/src"));

	std::set<FilePath> dirs = FileSystem::getSymLinkedDirectories(directoryPaths);

	REQUIRE(dirs.size() == 2);
}
