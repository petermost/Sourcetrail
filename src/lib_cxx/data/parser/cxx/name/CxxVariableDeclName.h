#ifndef CXX_VARIABLE_DECL_NAME_H
#define CXX_VARIABLE_DECL_NAME_H

#include <memory>
#include <vector>

#include "CxxDeclName.h"
#include "CxxTypeName.h"

class CxxVariableDeclName: public CxxDeclName
{
public:
	CxxVariableDeclName(
		std::string name,
		std::vector<std::string> templateParameterNames,
		std::unique_ptr<CxxTypeName> typeName,
		bool isStatic);

	NameHierarchy toNameHierarchy() const override;

private:
	std::unique_ptr<CxxTypeName> m_typeName;
	const bool m_isStatic;
};

#endif	  // CXX_VARIABLE_DECL_NAME_H
