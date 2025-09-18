#ifndef ACCESS_KIND_H
#define ACCESS_KIND_H

#include "utilityEnum.h"

#include <string>

// These values need to be the same as AccessKind in Java code (see AccessKind.java)
enum class AccessKind
{	 
	NONE = 0,
	PUBLIC = 1,
	PROTECTED = 2,
	PRIVATE = 3,
	DEFAULT = 4,
	TEMPLATE_PARAMETER = 5,
	TYPE_PARAMETER = 6
};

template <>
AccessKind intToEnum(int value);

std::string accessKindToString(AccessKind t);

#endif	  // ACCESS_KIND_H
