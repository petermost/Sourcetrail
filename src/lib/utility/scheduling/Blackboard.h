#ifndef BLACKBOARD_H
#define BLACKBOARD_H

#include "logging.h"

#include <aidkit/thread_shared.hpp>

#include <functional>
#include <map>
#include <memory>
#include <string>


struct BlackboardItemBase
{
	virtual ~BlackboardItemBase() = default;
};

template <typename T>
struct BlackboardItem: public BlackboardItemBase
{
	BlackboardItem(const T& v): value(v) {}

	~BlackboardItem() override = default;

	T value;
};

class Blackboard
{
public:
	Blackboard();
	Blackboard(std::shared_ptr<Blackboard> parent);

	// atomic way to set a blackboard value
	template <typename T>
	void set(const std::string& key, const T& value);

	// atomic way to get a blackboard value
	template <typename T>
	bool get(const std::string& key, T& value);

	// atomic way to update a blackboard value
	template <typename T>
	bool update(const std::string& key, std::function<T(const T&)> updater);

	bool exists(const std::string& key);
	bool clear(const std::string& key);

private:
	typedef std::map<std::string, std::shared_ptr<BlackboardItemBase>> ItemMap;

	std::shared_ptr<Blackboard> const m_parent;

	aidkit::thread_shared<ItemMap> m_items;
};


template <typename T>
void Blackboard::set(const std::string& key, const T& value)
{
	(*m_items.access())[key] = std::make_shared<BlackboardItem<T>>(value);
}

template <typename T>
bool Blackboard::get(const std::string& key, T& value)
{
	return aidkit::access([this, &key, &value](auto &items)
	{
		ItemMap::const_iterator it = items.find(key);
		if (it != items.end())
		{
			if (std::shared_ptr<BlackboardItem<T>> item = std::dynamic_pointer_cast<BlackboardItem<T>>(
					it->second))
			{
				value = item->value;
				return true;
			}
		}
		if (m_parent)
		{
			return m_parent->get(key, value);
		}

		LOG_WARNING("Entry for \"" + key + "\" not found on blackboard.");
		return false;
	}, m_items);
}

template <typename T>
bool Blackboard::update(const std::string& key, std::function<T(const T&)> updater)
{
	return aidkit::access([&key, &updater](auto &items)
	{
		ItemMap::const_iterator it = items.find(key);
		if (it != items.end())
		{
			if (std::shared_ptr<BlackboardItem<T>> item = std::dynamic_pointer_cast<BlackboardItem<T>>(
					it->second))
			{
				item->value = updater(item->value);
				return true;
			}
		}

		LOG_WARNING("Entry for \"" + key + "\" not found on blackboard.");
		return false;
	}, m_items);
}

#endif	  // BLACKBOARD_H
