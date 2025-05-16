#include "CxxDeclName.h"

CxxDeclName::CxxDeclName(std::string name): m_name(std::move(name)) {}

CxxDeclName::CxxDeclName(std::string name, std::vector<std::string> templateParameterNames)
	: m_name(std::move(name)), m_templateParameterNames(std::move(templateParameterNames))
{
}

NameHierarchy CxxDeclName::toNameHierarchy() const
{
	NameHierarchy ret = getParent() ? getParent()->toNameHierarchy()
									: NameHierarchy(NAME_DELIMITER_CXX);
	ret.push(m_name + getTemplateSuffix(m_templateParameterNames));
	return ret;
}

const std::string& CxxDeclName::getName() const
{
	return m_name;
}

const std::vector<std::string>& CxxDeclName::getTemplateParameterNames() const
{
	return m_templateParameterNames;
}
