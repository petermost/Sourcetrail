#include "Catch2.hpp"

#include "FilePathFilter.h"

TEST_CASE("file path filter finds exact match")
{
	FilePathFilter filter("test.h");

	REQUIRE(filter.isMatching(FilePath("test.h")));
}

TEST_CASE("file path filter finds match with single asterisk in same level")
{
	FilePathFilter filter("*test.*");

	REQUIRE(filter.isMatching(FilePath("this_is_a_test.h")));
}

TEST_CASE("file path filter finds match with single asterisk in different level")
{
	FilePathFilter filter("*/this_is_a_test.h");

	REQUIRE(filter.isMatching(FilePath("folder/this_is_a_test.h")));
}

TEST_CASE("file path filter does not find match with single asterisk in different level")
{
	FilePathFilter filter("*/test.h");

	REQUIRE(!filter.isMatching(FilePath("test.h")));
}

TEST_CASE("file path filter finds match with multiple asterisk in same level")
{
	FilePathFilter filter("**test.h");

	REQUIRE(filter.isMatching(FilePath("folder/this_is_a_test.h")));
}

TEST_CASE("file path filter finds match with multiple asterisk in different level")
{
	FilePathFilter filter("root/**/test.h");

	REQUIRE(filter.isMatching(FilePath("root/folder1/folder2/test.h")));
}

TEST_CASE("file path filter does not find match with multiple asterisk in different level")
{
	FilePathFilter filter("**/test.h");

	REQUIRE(!filter.isMatching(FilePath("folder/this_is_a_test.h")));
}

TEST_CASE("file path filter escapes dot character")
{
	FilePathFilter filter("test.h");

	REQUIRE(!filter.isMatching(FilePath("testyh")));
}

TEST_CASE("file path filter escapes plus character")
{
	REQUIRE(FilePathFilter("folder/test+.h").isMatching(FilePath("folder/test+.h")));
}

TEST_CASE("file path filter escapes minus character")
{
	REQUIRE(FilePathFilter("folder/test[-].h").isMatching(FilePath("folder/test[-].h")));
}

TEST_CASE("file path filter escapes dollar character")
{
	REQUIRE(FilePathFilter("folder/test$.h").isMatching(FilePath("folder/test$.h")));
}

TEST_CASE("file path filter escapes circumflex character")
{
	REQUIRE(FilePathFilter("folder/test^.h").isMatching(FilePath("folder/test^.h")));
}

TEST_CASE("file path filter escapes open round brace character")
{
	REQUIRE(FilePathFilter("folder/test(.h").isMatching(FilePath("folder/test(.h")));
}

TEST_CASE("file path filter escapes close round brace character")
{
	REQUIRE(FilePathFilter("folder\\test).h").isMatching(FilePath("folder/test).h")));
}

TEST_CASE("file path filter escapes open curly brace character")
{
	REQUIRE(FilePathFilter("folder/test{.h").isMatching(FilePath("folder/test{.h")));
}

TEST_CASE("file path filter escapes close curly brace character")
{
	REQUIRE(FilePathFilter("folder/test}.h").isMatching(FilePath("folder/test}.h")));
}

TEST_CASE("file path filter escapes open squared brace character")
{
	REQUIRE(FilePathFilter("folder/test[.h").isMatching(FilePath("folder/test[.h")));
}

TEST_CASE("file path filter escapes close squared brace character")
{
	REQUIRE(FilePathFilter("folder\\test].h").isMatching(FilePath("folder/test].h")));
}

TEST_CASE("file path filter finds backslash if slash was provided")
{
	FilePathFilter filter("folder/test.h");

	REQUIRE(filter.isMatching(FilePath("folder\\test.h")));
}

TEST_CASE("file path filter finds slash if backslash was provided")
{
	FilePathFilter filter("folder\\test.h");

	REQUIRE(filter.isMatching(FilePath("folder/test.h")));
}
