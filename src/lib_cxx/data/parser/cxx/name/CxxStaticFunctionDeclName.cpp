#include "CxxStaticFunctionDeclName.h"

CxxStaticFunctionDeclName::CxxStaticFunctionDeclName(
	std::string name,
	std::vector<std::string> templateParameterNames,
	std::unique_ptr<CxxTypeName> returnTypeName,
	std::vector<std::unique_ptr<CxxTypeName>> parameterTypeNames,
	std::string translationUnitFileName)
	: CxxFunctionDeclName(
		  std::move(name),
		  std::move(templateParameterNames),
		  std::move(returnTypeName),
		  std::move(parameterTypeNames),
		  false,
		  true)
	, m_translationUnitFileName(std::move(translationUnitFileName))
{
}

NameHierarchy CxxStaticFunctionDeclName::toNameHierarchy() const
{
	NameHierarchy ret = CxxFunctionDeclName::toNameHierarchy();
	NameElement& last = ret.back();
	last.setSignature(
		last.getSignature().getPrefix(),
		last.getSignature().getPostfix() + " (" + m_translationUnitFileName + ')');
	return ret;
}
