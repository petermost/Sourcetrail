#include "Catch2.hpp"

#include "utilityEnum.h"

#include <sstream>

using namespace std;
using namespace string_literals;

enum class Color : int{
	NONE = 0,
	
	RED = 1,
	GREEN = 2,
	BLUE = 3
};

const Color COLORS[] = {
	Color::NONE,
	Color::RED,
	Color::GREEN,
	Color::BLUE
};

template<>
Color intToEnum(int value)
{
	return lookupEnum(value, COLORS, Color::NONE);
}

TEST_CASE("enum lookup valid value")
{
	REQUIRE(intToEnum<Color>(2) == Color::GREEN);
}

TEST_CASE("enum lookup invalid value too small")
{
	REQUIRE(intToEnum<Color>(-1) == Color::NONE);
}

TEST_CASE("enum lookup invalid value too large")
{
	REQUIRE(intToEnum<Color>(4) == Color::NONE);
}

TEST_CASE("enum to_string")
{
	REQUIRE(to_string(Color::BLUE) == "3"s);
	REQUIRE(to_string(Color::BLUE) == "3"s);
}

TEST_CASE("enum stream operator")
{
	stringstream stream;
	stream << Color::RED;
	REQUIRE(stream.str() == "1"s);
	
	stringstream wstream;
	wstream << Color::RED;
	REQUIRE(wstream.str() == "1"s);
}

	
	
