#include "CxxSpecifierNameResolver.h"

#include "CxxDeclNameResolver.h"
#include "CxxTypeNameResolver.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/AST/PrettyPrinter.h>

CxxSpecifierNameResolver::CxxSpecifierNameResolver(CanonicalFilePathCache* canonicalFilePathCache)
	: CxxNameResolver(canonicalFilePathCache)
{
}

CxxSpecifierNameResolver::CxxSpecifierNameResolver(const CxxNameResolver* other)
	: CxxNameResolver(other)
{
}

std::unique_ptr<CxxName> CxxSpecifierNameResolver::getName(const clang::NestedNameSpecifier &nestedNameSpecifier)
{
	if (nestedNameSpecifier)
	{
		clang::NestedNameSpecifier::Kind nnsKind = nestedNameSpecifier.getKind();
		switch (nnsKind)
		{
		case clang::NestedNameSpecifier::Kind::Namespace:
			return CxxDeclNameResolver(this).getName(nestedNameSpecifier.getAsNamespaceAndPrefix().Namespace);

		case clang::NestedNameSpecifier::Kind::Type:
			return CxxTypeName::makeUnsolvedIfNull(CxxTypeNameResolver(this).getName(nestedNameSpecifier.getAsType()));

		case clang::NestedNameSpecifier::Kind::Global:
		case clang::NestedNameSpecifier::Kind::Null:
			// no context name hierarchy needed.
			break;

		case clang::NestedNameSpecifier::Kind::MicrosoftSuper:
			return CxxDeclNameResolver(this).getName(nestedNameSpecifier.getAsRecordDecl());
		}
	}

	return nullptr;
}
