#ifndef CXX_STATIC_FUNCTION_DECL_NAME_H
#define CXX_STATIC_FUNCTION_DECL_NAME_H

#include "CxxFunctionDeclName.h"

class CxxStaticFunctionDeclName: public CxxFunctionDeclName
{
public:
	CxxStaticFunctionDeclName(
		std::string name,
		std::vector<std::string> templateParameterNames,
		std::unique_ptr<CxxTypeName> returnTypeName,
		std::vector<std::unique_ptr<CxxTypeName>> parameterTypeNames,
		std::string translationUnitFileName);

	NameHierarchy toNameHierarchy() const override;

private:
	std::string m_translationUnitFileName;
};

#endif	  // CXX_FUNCTION_DECL_NAME_H
