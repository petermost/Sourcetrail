#include "AccessKind.h"

namespace
{

const AccessKind ACCESS_KINDS[] = {
	AccessKind::NONE,
	AccessKind::PUBLIC,
	AccessKind::PROTECTED,
	AccessKind::PRIVATE,
	AccessKind::DEFAULT,
	AccessKind::TEMPLATE_PARAMETER,
	AccessKind::TYPE_PARAMETER
};

}

template <>
AccessKind intToEnum(int value)
{
	return lookupEnum(value, ACCESS_KINDS, AccessKind::NONE);
}

std::string accessKindToString(AccessKind t)
{
	switch (t)
	{
	case AccessKind::NONE:
		return "";
	case AccessKind::PUBLIC:
		return "public";
	case AccessKind::PROTECTED:
		return "protected";
	case AccessKind::PRIVATE:
		return "private";
	case AccessKind::DEFAULT:
		return "default";
	case AccessKind::TEMPLATE_PARAMETER:
		return "template parameter";
	case AccessKind::TYPE_PARAMETER:
		return "type parameter";
	}
	return "";
}
