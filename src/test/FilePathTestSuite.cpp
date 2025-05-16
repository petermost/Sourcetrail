#include "Catch2.hpp"

#include "FilePath.h"
#include "Platform.h"

TEST_CASE("file_path_gets_created_empty")
{
	const FilePath path;

	REQUIRE(path.str() == "");
}

TEST_CASE("file_path_gets_created_with_string")
{
	const std::string str("data/FilePathTestSuite/main.cpp");
	const FilePath path(str);

	REQUIRE(path.str() == str);
}

TEST_CASE("file_path_gets_created_other_file_path")
{
	const FilePath path("data/FilePathTestSuite/main.cpp");
	const FilePath path2(path);

	REQUIRE(path == path2);
}

TEST_CASE("file_path_empty")
{
	const FilePath path1("data/FilePathTestSuite/a.cpp");
	const FilePath path2;

	REQUIRE(!path1.empty());
	REQUIRE(path2.empty());
}

TEST_CASE("file_path_exists")
{
	const FilePath path("data/FilePathTestSuite/a.cpp");

	REQUIRE(path.exists());
}

TEST_CASE("file_path_not_exists")
{
	const FilePath path("data/FilePathTestSuite/a.h");

	REQUIRE(!path.exists());
}

TEST_CASE("file_path_is_directory")
{
	const FilePath path("data/FilePathTestSuite/a.cpp");

	REQUIRE(!path.isDirectory());
	REQUIRE(path.getParentDirectory().isDirectory());
}

TEST_CASE("empty_file_path_has_empty_parent_directory")
{
	const FilePath path;

	REQUIRE(path.empty());
	REQUIRE(path.getParentDirectory().empty());
}

TEST_CASE("file_path_without_parent_has_empty_parent_directory")
{
	const FilePath path("a.cpp");

	REQUIRE(path.getParentDirectory().empty());
}

TEST_CASE("file_path_is_absolute")
{
	const FilePath path("data/FilePathTestSuite/a.cpp");

	REQUIRE(!path.isAbsolute());
	REQUIRE(path.getAbsolute().isAbsolute());
}

TEST_CASE("file_path_parent_directory")
{
	const FilePath path("data/FilePathTestSuite/a.cpp");

	REQUIRE(path.getParentDirectory().str() == "data/FilePathTestSuite");
	REQUIRE(path.getParentDirectory().getParentDirectory().str() == "data");
}

TEST_CASE("file_path_relative_to_other_path")
{
	const FilePath pathA("data/FilePathTestSuite/a.cpp");
	const FilePath pathB("data/FilePathTestSuite/test/c.h");

	REQUIRE(pathA.getRelativeTo(pathB).str() == "../a.cpp");
	REQUIRE(pathB.getRelativeTo(pathA).str() == "test/c.h");
}

TEST_CASE("file_path_relative_to_other_directory")
{
	const FilePath pathA("data/FilePathTestSuite/a.cpp");
	const FilePath pathB("data/FilePathTestSuite/test");

	REQUIRE(pathA.getRelativeTo(pathB).str() == "../a.cpp");
}

TEST_CASE("file_path_relative_to_same_directory")
{
	const FilePath pathA("data/FilePathTestSuite/test");

	REQUIRE(pathA.getRelativeTo(pathA).str() == "./");
}

TEST_CASE("file_path_file_name")
{
	const FilePath path("data/FilePathTestSuite/abc.h");

	REQUIRE(path.fileName() == "abc.h");
}

TEST_CASE("file_path_extension")
{
	const FilePath path("data/FilePathTestSuite/a.h");

	REQUIRE(path.extension() == ".h");
}

TEST_CASE("file_path_without_extension")
{
	const FilePath path("data/FilePathTestSuite/a.h");

	REQUIRE(path.withoutExtension() == FilePath("data/FilePathTestSuite/a"));
}

TEST_CASE("file_path_has_extension")
{
	std::vector<std::string> extensions;
	extensions.push_back(".h");
	extensions.push_back(".cpp");
	extensions.push_back(".cc");

	REQUIRE(FilePath("data/FilePathTestSuite/a.h").hasExtension(extensions));
	REQUIRE(FilePath("data/FilePathTestSuite/b.cpp").hasExtension(extensions));
	REQUIRE(!FilePath("data/FilePathTestSuite/a.m").hasExtension(extensions));
}

TEST_CASE("file_path_equals_file_with_different_relative_paths")
{
	const FilePath path1("data/FilePathTestSuite/a.cpp");
	const FilePath path2("data/../data/FilePathTestSuite/./a.cpp");

	REQUIRE(path1 == path2);
}

TEST_CASE("file_path_equals_relative_and_absolute_paths")
{
	const FilePath path1("data/FilePathTestSuite/a.cpp");
	const FilePath path2 = path1.getAbsolute();

	REQUIRE(path1 == path2);
}

TEST_CASE("file_path_equals_absolute_and_canonical_paths")
{
	const FilePath path("data/../data/FilePathTestSuite/./a.cpp");

	REQUIRE(path.getAbsolute() == path.getCanonical());
}

TEST_CASE("file_path_canonical_removes_symlinks")
{
	ASSERT_SYMLINK_PLATFORM();
	
	const FilePath pathA("data/FilePathTestSuite/parent/target/d.cpp");
	const FilePath pathB("data/FilePathTestSuite/target/d.cpp");
	
	REQUIRE(pathB.getAbsolute() == pathA.getCanonical());
}

TEST_CASE("file_path_compares_paths_with_posix_and_windows_format")
{
	ASSERT_BACKSLASH_PLATFORM();
		
	const FilePath pathB("data/FilePathTestSuite/b.cc");
	const FilePath pathB2("data\\FilePathTestSuite\\b.cc");
	
	REQUIRE(pathB == pathB2);
}
	
TEST_CASE("file_path_differs_for_different_existing_files")
{
	const FilePath pathA("data/FilePathTestSuite/a.cpp");
	const FilePath pathB("data/FilePathTestSuite/b.cc");

	REQUIRE(pathA != pathB);
}

TEST_CASE("file_path_differs_for_different_nonexistent_files")
{
	const FilePath pathA("data/FilePathTestSuite/a.h");
	const FilePath pathB("data/FilePathTestSuite/b.c");

	REQUIRE(pathA != pathB);
}

TEST_CASE("file_path_differs_for_existing_and_nonexistent_files")
{
	const FilePath pathA("data/FilePathTestSuite/a.h");
	const FilePath pathB("data/FilePathTestSuite/b.cc");

	REQUIRE(pathA != pathB);
}

TEST_CASE("file path is valid for absolute and relative existing files and directories paths")
{
	REQUIRE(FilePath("data/FilePathTestSuite/a.cpp").isValid());
	REQUIRE(FilePath("data/FilePathTestSuite/a.cpp").makeAbsolute().isValid());
	REQUIRE(FilePath("data/FilePathTestSuite/with space/s.srctrlprj").isValid());
	REQUIRE(FilePath("data/FilePathTestSuite/with space/s.srctrlprj").makeAbsolute().isValid());
	REQUIRE(FilePath("data/FilePathTestSuite").isValid());
	REQUIRE(FilePath("data/FilePathTestSuite").makeAbsolute().isValid());
	REQUIRE(FilePath("data/FilePathTestSuite/container.app").isValid());
	REQUIRE(FilePath("data/FilePathTestSuite/container.app").makeAbsolute().isValid());
	REQUIRE(FilePath("data/FilePathTestSuite/container.app/b.txt").isValid());
	REQUIRE(FilePath("data/FilePathTestSuite/container.app/b.txt").makeAbsolute().isValid());
}

TEST_CASE("file path is valid for absolute and relative non-existing files and directories paths")
{
	REQUIRE(FilePath("data/non-existing-file.cpp").isValid());
	REQUIRE(FilePath("data/non-existing-file.cpp").makeAbsolute().isValid());
	REQUIRE(FilePath("data/non-existing-dir").isValid());
	REQUIRE(FilePath("data/non-existing-dir").makeAbsolute().isValid());
	REQUIRE(FilePath("data/FilePathTestSuite/container.app/c.txt").isValid());
	REQUIRE(FilePath("data/FilePathTestSuite/container.app/c.txt").makeAbsolute().isValid());
	REQUIRE(FilePath("data/FilePathTestSuite/also space").isValid());
	REQUIRE(FilePath("data/FilePathTestSuite/also space").makeAbsolute().isValid());
}

TEST_CASE("file path is invalid for absolute and relative paths with invalid characters")
{
	REQUIRE(!FilePath("data/non-exis\"ting-file.cpp").isValid());
	REQUIRE(!FilePath("data/non-exis\"ting-file.cpp").makeAbsolute().isValid());
	REQUIRE(!FilePath("data/non-exis|ting-dir").isValid());
	REQUIRE(!FilePath("data/non-exis|ting-dir").makeAbsolute().isValid());
	REQUIRE(!FilePath("data/FilePathTestSuite/container:app").isValid());
	REQUIRE(!FilePath("data/FilePathTestSuite/container:app").makeAbsolute().isValid());
}
