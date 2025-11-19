// Copyright 2015 Peter Most, PERA Software Solutions GmbH
//
// This file is part of the CppAidKit library.
//
// CppAidKit is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CppAidKit is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with CppAidKit. If not, see <http://www.gnu.org/licenses/>.

#include <gtest/gtest.h>
#include <aidkit/shared_data.hpp>
#include <string>
#include <vector>

using namespace std;
using namespace aidkit;

//#########################################################################################################

using SharedStringVectorData = shared_data<vector<string>>;

// Explicit template instantiation to detect syntax errors:
template class aidkit::shared_data<vector<string>>;

TEST(SharedDataTest, testRegularLocking)
{
	SharedStringVectorData names(20u, "empty");

	shared_data_ptr names_ptr(names.lock());
	names_ptr->at(0) = "";
	(*names_ptr)[0] = "";

	ASSERT_EQ(names_ptr->at(0), "");
}

TEST(SharedDataTest, testConstLocking)
{
	const SharedStringVectorData names(20u, "empty");

	const_shared_data_ptr names_ptr(names.lock());
	string first = names_ptr->at(0);
	first = (*names_ptr)[0];

	ASSERT_EQ(names_ptr->at(0), "empty");
}
