#ifndef UTILITY_ENUM_H
#define UTILITY_ENUM_H

#include <limits>
#include <ostream>
#include <string>
#include <type_traits>

template <typename E>
concept EnumType = std::is_enum_v<E>;

template <EnumType E>
constexpr auto MAX_ENUM_VALUE = std::numeric_limits<std::underlying_type_t<E>>::max();

template <EnumType E>
E intToEnum(int);

template <EnumType E, std::size_t N>
E lookupEnum(int i, const E (&enums)[N], E &&defaultValue)
{
	if (i >= 0 && static_cast<std::size_t>(i) < N)
		return enums[i];
	else
		return defaultValue;
}

template <EnumType E>
std::string to_string(E e)
{
	return std::to_string(static_cast<std::underlying_type_t<E>>(e));
}

template <EnumType E>
std::ostream &operator << (std::ostream &stream, E e)
{
	return stream << static_cast<std::underlying_type_t<E>>(e);
}

#endif
