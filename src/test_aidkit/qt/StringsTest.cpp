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

#include <aidkit/qt/Strings.hpp>

#include <sstream>

using namespace std;
using namespace aidkit::qt;

TEST(QStringsTest, charLiteralOperator)
{
	QChar c = 'z'_qc;

	ASSERT_EQ(c, 'z');
}

TEST(QStringsTest, stringLiteralOperator)
{
	QString s = "string"_qs;

	ASSERT_EQ(s, "string");
}

TEST(QStringsTest, streamOperator)
{
	ostringstream stream;
	stream << "QString"_qs;

	ASSERT_EQ(stream.str(), "QString");
}


