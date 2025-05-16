#include "Id.h"

#include <ostream>

using namespace std;

ostream &operator << (ostream &os, const Id id)
{
	return os << static_cast<Id::type>(id);
}

string to_string(const Id id)
{
	return std::to_string(static_cast<Id::type>(id));
}

namespace std
{

size_t hash<Id>::operator()(const Id id) const noexcept
{
	return m_hash(static_cast<Id::type>(id));
}

}
