#ifndef CXX_NAME_H
#define CXX_NAME_H

#include <memory>
#include <string>
#include <vector>

#include "NameHierarchy.h"

class CxxName
{
public:
	CxxName();
	CxxName(std::shared_ptr<CxxName> parent);

	virtual ~CxxName() = default;

	void setParent(std::shared_ptr<CxxName> parent);
	std::shared_ptr<CxxName> getParent() const;

	static std::string getTemplateSuffix(const std::vector<std::string>& elements);

	virtual NameHierarchy toNameHierarchy() const = 0;

private:
	std::shared_ptr<CxxName> m_parent;
};

#endif	  // CXX_NAME_H
