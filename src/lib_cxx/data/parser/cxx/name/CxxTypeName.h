#ifndef CXX_TYPE_NAME_H
#define CXX_TYPE_NAME_H

#include <memory>
#include <string>
#include <vector>

#include "CxxName.h"
#include "CxxQualifierFlags.h"
#include "NameHierarchy.h"

class CxxTypeName: public CxxName
{
public:
	static std::unique_ptr<CxxTypeName> getUnsolved();
	static std::unique_ptr<CxxTypeName> makeUnsolvedIfNull(std::unique_ptr<CxxTypeName> name);

	struct Modifier
	{
		Modifier(std::string symbol);

		const std::string symbol;
		CxxQualifierFlags qualifierFlags;
	};

	CxxTypeName(std::string name);

	CxxTypeName(std::string name, std::vector<std::string> templateArguments);

	CxxTypeName(
		std::string name,
		std::vector<std::string> templateArguments,
		std::shared_ptr<CxxName> parent);

	NameHierarchy toNameHierarchy() const override;

	void addQualifier(const CxxQualifierFlags::QualifierType qualifier);
	void addModifier(Modifier modifier);

	std::string toString() const;

private:
	const std::string m_name;
	const std::vector<std::string> m_templateArguments;

	CxxQualifierFlags m_qualifierFlags;
	std::vector<Modifier> m_modifiers;
};

#endif	  // CXX_TYPE_NAME_H
