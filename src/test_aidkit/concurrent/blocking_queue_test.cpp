// Copyright 2023 Peter Most, PERA Software Solutions GmbH
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

#include <aidkit/concurrent/blocking_queue.hpp>

#include <chrono>
#include <string>

using namespace std;
using namespace aidkit::concurrent;

//#########################################################################################################

// Explicit template instantiation to detect syntax errors:
template class aidkit::concurrent::blocking_queue<string>;

TEST(BlockingQueueTest, testEnqueue)
{
	const string expectedName("name");

	blocking_queue<string> names;

	names.enqueue(expectedName);
	string actualName = names.dequeue();

	ASSERT_EQ(expectedName, actualName);
}

TEST(BlockingQueueTest, testEnqueueMove)
{
	const string expectedName("name");
	string duplicatedName(expectedName);

	blocking_queue<string> names;

	names.enqueue(std::move(duplicatedName));
	string actualName = names.dequeue();

	ASSERT_EQ(expectedName, actualName);
}

TEST(BlockingQueueTest, testDequeueTimeout)
{
	const string expectedName("name");

	blocking_queue<string> names;

	names.enqueue(expectedName);
	optional<string> actualName = names.dequeue(0ms);

	ASSERT_EQ(actualName.has_value(), true);
	ASSERT_EQ(actualName.value(), expectedName);
}

TEST(BlockingQueueTest, testDequeueTimeoutFailed)
{
	blocking_queue<string> names;

	optional<string> actualName = names.dequeue(0ms);

	ASSERT_EQ(actualName.has_value(), false);
}

