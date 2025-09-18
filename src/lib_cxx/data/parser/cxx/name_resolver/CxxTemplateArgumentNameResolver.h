#ifndef CXX_TEMPLATE_ARGUMENT_NAME_RESOLVER_H
#define CXX_TEMPLATE_ARGUMENT_NAME_RESOLVER_H

#include <memory>

#include "CxxNameResolver.h"

class DataType;

class CxxTemplateArgumentNameResolver: public CxxNameResolver
{
public:
	CxxTemplateArgumentNameResolver(const CxxNameResolver* other);

	std::string getTemplateArgumentName(const clang::TemplateArgument& argument);
};

#endif	  // CXX_TEMPLATE_ARGUMENT_NAME_RESOLVER_H
