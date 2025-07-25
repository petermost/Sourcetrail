#include "Blackboard.h"

Blackboard::Blackboard() = default;

Blackboard::Blackboard(std::shared_ptr<Blackboard> parent)
	: m_parent(parent)
{
}

bool Blackboard::exists(const std::string& key)
{
	return aidkit::access([&key](auto &items)
	{
		ItemMap::const_iterator it = items.find(key);
		return (it != items.end());
	}, m_items);
}

bool Blackboard::clear(const std::string& key)
{
	return aidkit::access([&key](auto &items)
	{
		ItemMap::const_iterator it = items.find(key);
		if (it != items.end())
		{
			items.erase(it);
			return true;
		}
		return false;
	}, m_items);
}
