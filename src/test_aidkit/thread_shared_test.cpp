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

#include <aidkit/thread_shared.hpp>

using namespace std;
using namespace aidkit;

//#########################################################################################################

// We are using this class and not vector or string, so the template error messages are easier to decipher:

class Data {
	public:
		Data(int v)
			: m_value(v)
		{
		}

		void set(int v)
		{
			m_value = v;
		}

		int get() const
		{
			return m_value;
		}

		bool operator == (const Data &other) const
		{
			return m_value == other.m_value;
		}
	private:
		int m_value = 0;
};


// Explicit template instantiation to detect syntax errors early:
template class aidkit::thread_shared<Data>;

TEST(ThreadSharedTest, testAccess)
{
	thread_shared<Data> sharedData(20);

	auto dataAccess = sharedData.access();
	ASSERT_EQ(dataAccess->get(), 20);

	dataAccess->set(10);
	ASSERT_EQ(dataAccess->get(), 10);
}

TEST(ThreadSharedTest, testConstAccess)
{
	const thread_shared<Data> constSharedData(20);

	auto dataAccess = constSharedData.access();
	ASSERT_EQ(dataAccess->get(), 20);

	dataAccess.unlock();
	dataAccess.lock();

	// /* Must not compile: */ dataAccess->set(10);
}

TEST(ThreadSharedTest, testAccessFunction)
{
	thread_shared<Data> sharedData(20);
	ASSERT_EQ(sharedData.access()->get(), 20);

	access([](Data &c)
	{
		ASSERT_EQ(c.get(), 20);
	}, sharedData);

	access([](Data &c)
	{
		c.set(10);
	}, sharedData);
	ASSERT_EQ(sharedData.access()->get(), 10);
}

TEST(ThreadSharedTest, testMultipleAccessFunction)
{
	thread_shared<Data> sharedData10(10);
	thread_shared<Data> sharedData20(20);

	access([](Data &d1, Data &d2)
	{
		std::swap(d1, d2);
	}, sharedData20, sharedData10);

	ASSERT_EQ(sharedData10.access()->get(), 20);
	ASSERT_EQ(sharedData20.access()->get(), 10);
}

TEST(ThreadSharedTest, testConstAccessFunction)
{
	const thread_shared<Data> sharedData(20);
	ASSERT_EQ(sharedData.access()->get(), 20);

	access([](const Data &c)
	{
		ASSERT_EQ(c.get(), 20);
	}, sharedData);

	// Must not compile:
	// access([](const Data &c)
	// {
	// 	c.set(10);
	// }, sharedData);
}

TEST(ThreadSharedTest, testAssignConversion)
{
	thread_shared<Data> sharedData(15);
	Data otherData(30);

	sharedData = otherData;
	Data copyData = sharedData;

	ASSERT_EQ(copyData, otherData);
}
