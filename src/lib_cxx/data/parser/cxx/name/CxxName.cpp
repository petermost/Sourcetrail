#include "CxxName.h"

#include "utilityString.h"

CxxName::CxxName() = default;

CxxName::CxxName(std::shared_ptr<CxxName> parent): m_parent(parent) {}

void CxxName::setParent(std::shared_ptr<CxxName> parent)
{
	m_parent = parent;
}

std::shared_ptr<CxxName> CxxName::getParent() const
{
	return m_parent;
}

std::string CxxName::getTemplateSuffix(const std::vector<std::string>& elements) 
{
	if (elements.size())
	{
		return '<' + utility::join(elements, ", ") + '>';
	}

	return "";
}
