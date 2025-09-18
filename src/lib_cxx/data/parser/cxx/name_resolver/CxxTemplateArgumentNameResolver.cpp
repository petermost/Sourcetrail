#include "CxxTemplateArgumentNameResolver.h"

#include "CxxTypeNameResolver.h"
#include "utilityClang.h"

#include <clang/AST/DeclTemplate.h>
#include <clang/AST/PrettyPrinter.h>

using namespace utility;

CxxTemplateArgumentNameResolver::CxxTemplateArgumentNameResolver(const CxxNameResolver* other)
	: CxxNameResolver(other)
{
}

std::string CxxTemplateArgumentNameResolver::getTemplateArgumentName(
	const clang::TemplateArgument& argument)
{
	// This doesn't work correctly if the template argument is dependent.
	// If that's required: build name from depth and index of template arg.
	const clang::TemplateArgument::ArgKind kind = argument.getKind();
	switch (kind)
	{
	case clang::TemplateArgument::Type:
	{
		CxxTypeNameResolver typeNameResolver(this);
		std::unique_ptr<CxxTypeName> typeName = CxxTypeName::makeUnsolvedIfNull(
			typeNameResolver.getName(argument.getAsType()));
		return typeName->toString();
	}
	case clang::TemplateArgument::Integral:
	case clang::TemplateArgument::Null:
	case clang::TemplateArgument::Declaration:
	case clang::TemplateArgument::NullPtr:
	case clang::TemplateArgument::Template:
	case clang::TemplateArgument::TemplateExpansion:	// handled correctly? template template parameter...
	case clang::TemplateArgument::Expression:
	case clang::TemplateArgument::StructuralValue:
	{
		clang::PrintingPolicy pp = makePrintingPolicyForCPlusPlus();

		constexpr bool includeType = false;
		std::string buf;
		llvm::raw_string_ostream os(buf);
		argument.print(pp, os, includeType);
		return os.str();
	}
	case clang::TemplateArgument::Pack:
	{
		return "<...>";
	}
	}

	return "";
}
