#include "Catch2.hpp"

#include "utilityStl.h"

#include <map>

using namespace std;
using namespace utility;

TEST_CASE("find_optional for container succeeding")
{
	const map<int, char> intChars = {
		{ 1, '1' },
		{ 2, '2' },
		{ 3, '3' }
	};

	auto optionalIterator = find_optional(intChars, 3);

	REQUIRE(optionalIterator);
	REQUIRE((*optionalIterator)->second == '3');
}

TEST_CASE("find_optional for container failing")
{
	const map<int, char> intChars = {
		{ 1, '1' },
		{ 2, '2' },
		{ 3, '3' }
	};

	auto optionalIterator = find_optional(intChars, 10);
	REQUIRE(!optionalIterator);
	REQUIRE(optionalIterator == nullopt);
}
