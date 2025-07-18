#ifndef UTILITY_STL_H
#define UTILITY_STL_H

#include <optional>

namespace utility
{

template <typename Container, typename T>
std::optional<typename Container::const_iterator> find_optional(const Container &container, const T &value)
{
	auto it = container.find(value);

	if (it != container.end())
		return std::optional(it);
	else
		return std::nullopt;
}

}

#endif
