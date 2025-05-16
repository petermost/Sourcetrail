#ifndef GROUP_TYPE_H
#define GROUP_TYPE_H

#include <string>

enum class GroupType
{
	NONE,
	DEFAULT,
	FRAMELESS,
	FILE,
	NAMESPACE,
	INHERITANCE
};

std::string groupTypeToString(GroupType type);
GroupType stringToGroupType(const std::string& value);

enum class GroupLayout
{
	LIST,
	SKEWED,
	BUCKET,
	SQUARE
};

#endif	  // GROUP_TYPE_H
