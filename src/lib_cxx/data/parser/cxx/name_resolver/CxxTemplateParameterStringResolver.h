#ifndef CXX_TEMPLATE_PARAMETER_STRING_RESOLVER_H
#define CXX_TEMPLATE_PARAMETER_STRING_RESOLVER_H

#include <memory>

#include <clang/AST/DeclTemplate.h>

#include "CxxNameResolver.h"

class DataType;

class CxxTemplateParameterStringResolver: public CxxNameResolver
{
public:
	CxxTemplateParameterStringResolver(CanonicalFilePathCache* canonicalFilePathCache);
	CxxTemplateParameterStringResolver(const CxxNameResolver* other);

	std::string getTemplateParameterString(const clang::NamedDecl* parameter);
	std::string getTemplateParameterTypeString(const clang::NonTypeTemplateParmDecl* parameter);
	static std::string getTemplateParameterTypeString(const clang::TemplateTypeParmDecl* parameter);
	std::string getTemplateParameterTypeString(const clang::TemplateTemplateParmDecl* parameter);
};

#endif	  // CXX_TEMPLATE_PARAMETER_STRING_RESOLVER_H
