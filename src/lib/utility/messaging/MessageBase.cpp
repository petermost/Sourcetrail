#include "MessageBase.h"

#include <boost/core/demangle.hpp>

Id MessageBase::s_nextId = 1;

std::ostream &operator << (std::ostream &stream, const std::type_info &info)
{
	return stream << boost::core::demangle(info.name());
}
