#ifndef CXX_DECL_NAME_H
#define CXX_DECL_NAME_H

#include <memory>
#include <string>
#include <vector>

#include "CxxName.h"
#include "NameHierarchy.h"

class CxxDeclName: public CxxName
{
public:
	CxxDeclName(std::string name);

	CxxDeclName(std::string name, std::vector<std::string> templateParameterNames);

	NameHierarchy toNameHierarchy() const override;

	const std::string& getName() const;
	const std::vector<std::string>& getTemplateParameterNames() const;

private:
	const std::string m_name;
	const std::vector<std::string> m_templateParameterNames;
};

#endif	  // CXX_DECL_NAME_H
