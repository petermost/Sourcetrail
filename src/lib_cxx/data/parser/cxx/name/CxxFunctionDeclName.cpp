#include "CxxFunctionDeclName.h"

#include <sstream>

CxxFunctionDeclName::CxxFunctionDeclName(
	std::string name,
	std::vector<std::string> templateParameterNames,
	std::unique_ptr<CxxTypeName> returnTypeName,
	std::vector<std::unique_ptr<CxxTypeName>> parameterTypeNames,
	const bool isConst,
	const bool isStatic)
	: CxxDeclName(std::move(name), std::move(templateParameterNames))
	, m_returnTypeName(std::move(returnTypeName))
	, m_parameterTypeNames(std::move(parameterTypeNames))
	, m_isConst(isConst)
	, m_isStatic(isStatic)
{
}

NameHierarchy CxxFunctionDeclName::toNameHierarchy() const
{
	std::stringstream prefix;
	if (m_isStatic)
	{
		prefix << "static ";
	}
	prefix << m_returnTypeName->toString();

	std::stringstream postfix;
	postfix << '(';
	for (size_t i = 0; i < m_parameterTypeNames.size(); i++)
	{
		if (i != 0)
		{
			postfix << ", ";
		}
		postfix << m_parameterTypeNames[i]->toString();
	}
	postfix << ')';
	if (m_isConst)
	{
		postfix << " const";
	}

	NameHierarchy ret = CxxDeclName::toNameHierarchy();
	ret.back().setSignature(prefix.str(), postfix.str());
	return ret;
}
