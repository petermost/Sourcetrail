#include "CxxTemplateArgumentNameResolver.h"

#include <sstream>

#include <clang/AST/DeclTemplate.h>
#include <clang/AST/PrettyPrinter.h>

#include "CxxTypeNameResolver.h"
#include "utilityClang.h"
#include "utilityString.h"

using namespace utility;

CxxTemplateArgumentNameResolver::CxxTemplateArgumentNameResolver(
	CanonicalFilePathCache* canonicalFilePathCache)
	: CxxNameResolver(canonicalFilePathCache)
{
}

CxxTemplateArgumentNameResolver::CxxTemplateArgumentNameResolver(const CxxNameResolver* other)
	: CxxNameResolver(other)
{
}

std::wstring CxxTemplateArgumentNameResolver::getTemplateArgumentName(
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
<<<<<<< HEAD
		argument.print(pp, os, includeType);
=======
		argument.print(pp, os, true); // 3rd argument is required with llvm ver. >=14
>>>>>>> 09ccbe42a1120f7185e91e13d9d2b8583217be7f
		return utility::decodeFromUtf8(os.str());
	}
	case clang::TemplateArgument::Pack:
	{
		return L"<...>";
	}
	}

	return L"";
}
