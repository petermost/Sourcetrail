#ifndef CXX_DECL_NAME_RESOLVER_H
#define CXX_DECL_NAME_RESOLVER_H

#include <clang/AST/DeclTemplate.h>

#include "CxxDeclName.h"
#include "CxxNameResolver.h"

class CanonicalFilePathCache;

class CxxDeclNameResolver: public CxxNameResolver
{
public:
	CxxDeclNameResolver(CanonicalFilePathCache* canonicalFilePathCache);
	CxxDeclNameResolver(const CxxNameResolver* other);

	std::unique_ptr<CxxDeclName> getName(const clang::NamedDecl* declaration);

private:
	std::unique_ptr<CxxName> getContextName(const clang::DeclContext* declaration);
	std::unique_ptr<CxxDeclName> getDeclName(const clang::NamedDecl* declaration);
	std::string getTranslationUnitMainFileName(const clang::Decl* declaration);
	std::string getNameForAnonymousSymbol(const std::string& symbolKindName, const clang::Decl* declaration);
	std::vector<std::string> getTemplateParameterStrings(const clang::TemplateDecl* templateDecl);
	template <typename T>
	std::vector<std::string> getTemplateParameterStringsOfPartialSpecialization(const T* templateDecl);
	std::string getTemplateParameterString(const clang::NamedDecl* parameter);
	std::string getTemplateArgumentName(const clang::TemplateArgument& argument);

	const clang::NamedDecl* m_currentDecl;
};

#endif
