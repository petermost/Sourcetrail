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

import aidkit.enum_class;

#include <gtest/gtest.h>

using namespace std;
using namespace aidkit;

//#########################################################################################################

// Enum for testing default value assignment:

class Color : public aidkit::enum_class<Color> {
	public:
		static const Color Red;
		static const Color Green;
		static const Color Blue;

	private:
		using enum_class::enum_class;
};

const Color Color::Red("Red");
const Color Color::Green("Green");
const Color Color::Blue("Blue");

static void colorFunction(Color)
{
}

// Explicite template instantiation to detect syntax errors:

template class aidkit::enum_class<Color>;

TEST(ColorEnumTest, testCallByValue)
{
	colorFunction(Color::Blue);
}

TEST(ColorEnumTest, testValue)
{
	ASSERT_EQ(Color::Red.value(), 0);
	ASSERT_EQ(Color::Green.value(), 1);
	ASSERT_EQ(Color::Blue.value(), 2);
}

TEST(ColorEnumTest, testValues)
{
	vector<Color> colors;
	Color::for_each([&](const Color &color) {
		colors.push_back(color);
	});
	ASSERT_EQ(colors.size(), static_cast<size_t>(3));
}

TEST(ColorEnumTest, testName)
{
	ASSERT_EQ(Color::Red.name(), "Red");
	ASSERT_EQ(Color::Green.name(), "Green");
	ASSERT_EQ(Color::Blue.name(), "Blue");
}

TEST(ColorEnumTest, testFindByValue)
{
	vector<Color> result = Color::find(2);
	ASSERT_EQ(result.size(), static_cast<size_t>(1));
	ASSERT_EQ(result[0], Color::Blue);
}

TEST(ColorEnumTest, testFindByName)
{
	vector<Color> result = Color::find("Green");
	ASSERT_EQ(result.size(), static_cast<size_t>(1));
	ASSERT_EQ(result[0], Color::Green);
}

TEST(ColorEnumTest, testEquality)
{
	ASSERT_EQ(Color::Red, Color::Red);
	ASSERT_EQ(Color::Green, Color::Green);
	ASSERT_EQ(Color::Blue, Color::Blue);
}

TEST(ColorEnumTest, testLessThan)
{
	ASSERT_LT(Color::Red, Color::Green);
	ASSERT_LT(Color::Green, Color::Blue);
}

TEST(ColorEnumTest, testAssignment)
{
	Color color = Color::Red;
	ASSERT_EQ(color, Color::Red);

	color = Color::Blue;
	ASSERT_EQ(color, Color::Blue);
}

//##################################################################################################

// Enum for testing explicit value assignment and duplicated enums:

class Number : public enum_class<Number> {
	public:
		static const Number Ten;
		static const Number Twenty;
		static const Number Thirty;
		static const Number TwentyToo;

	private:
		using enum_class::enum_class;
};

const Number Number::Ten(10, "Ten");
const Number Number::Twenty(20, "Twenty");
const Number Number::Thirty(30, "Thirty");
const Number Number::TwentyToo(20, "TwentyToo");

TEST(NumberEnumTest, testValue)
{
	ASSERT_EQ(Number::Ten.value(), 10);
	ASSERT_EQ(Number::Twenty.value(), 20);
	ASSERT_EQ(Number::Thirty.value(), 30);
	ASSERT_EQ(Number::TwentyToo.value(), 20);
}

TEST(NumberEnumTest, testFindDuplicates)
{
	vector<Number> numbers = Number::find(20);
	ASSERT_EQ(numbers.size(), static_cast<size_t>(2));
	ASSERT_EQ(numbers[0], Number::Twenty);
	ASSERT_EQ(numbers[1], Number::TwentyToo);
}

//#########################################################################################################

// Enum for testing explicit start value:

class Animal : public enum_class<Animal> {
	public:
		static const Animal Cat;
		static const Animal Dog;

	private:
		using enum_class::enum_class;
};

const Animal Animal::Cat(10);
const Animal Animal::Dog;

TEST(AnimalEnumTest, testValue)
{
	ASSERT_EQ(Animal::Cat.value(), 10);
	ASSERT_EQ(Animal::Dog.value(), 11);
}

TEST(AnimalEnumTest, testName)
{
	ASSERT_EQ(Animal::Cat.name(), "");
	ASSERT_EQ(Animal::Dog.name(), "");
}
