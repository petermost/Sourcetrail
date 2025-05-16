#include "TokenComponentAccess.h"

std::string TokenComponentAccess::getAccessString(AccessKind access)
{
	switch (access)
	{
	case AccessKind::NONE:
		break;
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


TokenComponentAccess::TokenComponentAccess(AccessKind access): m_access(access) {}

TokenComponentAccess::~TokenComponentAccess() = default;

std::shared_ptr<TokenComponent> TokenComponentAccess::copy() const
{
	return std::make_shared<TokenComponentAccess>(*this);
}

AccessKind TokenComponentAccess::getAccess() const
{
	return m_access;
}

std::string TokenComponentAccess::getAccessString() const
{
	return getAccessString(m_access);
}
