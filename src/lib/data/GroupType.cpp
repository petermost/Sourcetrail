#include "GroupType.h"

std::string groupTypeToString(GroupType type)
{
	switch (type)
	{
	case GroupType::NONE:
		return "none";
	case GroupType::DEFAULT:
		return "default";
	case GroupType::FRAMELESS:
		return "frameless";
	case GroupType::FILE:
		return "file";
	case GroupType::NAMESPACE:
		return "namespace";
	case GroupType::INHERITANCE:
		return "inheritance";
	}

	return "none";
}

GroupType stringToGroupType(const std::string& value)
{
	if (value == groupTypeToString(GroupType::NONE))
		return GroupType::NONE;
	if (value == groupTypeToString(GroupType::DEFAULT))
		return GroupType::DEFAULT;
	if (value == groupTypeToString(GroupType::FRAMELESS))
		return GroupType::FRAMELESS;
	if (value == groupTypeToString(GroupType::FILE))
		return GroupType::FILE;
	if (value == groupTypeToString(GroupType::NAMESPACE))
		return GroupType::NAMESPACE;
	if (value == groupTypeToString(GroupType::INHERITANCE))
		return GroupType::INHERITANCE;

	return GroupType::NONE;
}
