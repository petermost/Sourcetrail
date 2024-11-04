#include "utilityClang.h"

#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/Basic/FileManager.h>
#include <clang/Lex/Preprocessor.h>

#include "CanonicalFilePathCache.h"
#include "FilePath.h"
#include "ParseLocation.h"
#include "utilityString.h"

using namespace std;
using namespace clang;

bool utility::isImplicit(const clang::Decl* d)
{
	if (!d)
	{
		return false;
	}

	if (d->isImplicit())
	{
		if (const clang::RecordDecl* rd = clang::dyn_cast_or_null<clang::RecordDecl>(d))
		{
			if (rd->isLambda())
			{
				return isImplicit(clang::dyn_cast_or_null<clang::Decl>(d->getDeclContext()));
			}
		}
		return true;
	}
	else if (
		const clang::ClassTemplateSpecializationDecl* ctsd =
			clang::dyn_cast_or_null<clang::ClassTemplateSpecializationDecl>(d))
	{
		if (!ctsd->isExplicitSpecialization())
		{
			return true;
		}
	}
	else if (const clang::FunctionDecl* fd = clang::dyn_cast_or_null<clang::FunctionDecl>(d))
	{
		if (fd->isTemplateInstantiation() &&
			fd->getTemplateSpecializationKind() !=
				clang::TSK_ExplicitSpecialization)	  // or undefined??
		{
			return true;
		}
		else if (
			fd->getMemberSpecializationInfo() &&
			fd->getMemberSpecializationInfo()->getTemplateSpecializationKind() ==
				clang::TSK_ExplicitSpecialization)
		{
			return false;
		}
	}

	return isImplicit(clang::dyn_cast_or_null<clang::Decl>(d->getDeclContext()));
}

AccessKind utility::convertAccessSpecifier(clang::AccessSpecifier access)
{
	switch (access)
	{
	case clang::AS_public:
		return ACCESS_PUBLIC;
	case clang::AS_protected:
		return ACCESS_PROTECTED;
	case clang::AS_private:
		return ACCESS_PRIVATE;
	case clang::AS_none:
		return ACCESS_NONE;
	default:
		return ACCESS_NONE;
	}
}

SymbolKind utility::convertTagKind(const clang::TagTypeKind tagKind)
{
	switch (tagKind)
	{
	case TagTypeKind::Struct:
		return SYMBOL_STRUCT;
	case TagTypeKind::Union:
		return SYMBOL_UNION;
	case TagTypeKind::Class:
		return SYMBOL_CLASS;
	case TagTypeKind::Enum:
		return SYMBOL_ENUM;
	case TagTypeKind::Interface:
		return SYMBOL_KIND_MAX;
	default:
		return SYMBOL_KIND_MAX;
	}
}

<<<<<<< HEAD
bool utility::isLocalVariable(const clang::ValueDecl *d)
=======
bool utility::isLocalVariable(const clang::ValueDecl* d)
>>>>>>> 09ccbe42a1120f7185e91e13d9d2b8583217be7f
{
	return !llvm::isa<clang::ParmVarDecl>(d) && !(d->getParentFunctionOrMethod() == nullptr);
}

<<<<<<< HEAD
bool utility::isParameter(const clang::ValueDecl *d)
=======
bool utility::isParameter(const clang::ValueDecl* d)
>>>>>>> 09ccbe42a1120f7185e91e13d9d2b8583217be7f
{
	return llvm::isa<clang::ParmVarDecl>(d);
}

SymbolKind utility::getSymbolKind(const clang::VarDecl* d)
{
	SymbolKind symbolKind = SYMBOL_KIND_MAX;

	if (d->getParentFunctionOrMethod() == nullptr)
	{
		if (d->getAccess() == clang::AS_none)
		{
			symbolKind = SYMBOL_GLOBAL_VARIABLE;
		}
		else
		{
			symbolKind = SYMBOL_FIELD;
		}
	}

	return symbolKind;
}

wstring utility::getFileNameOfFileEntry(const FileEntryRef &entry)
{
<<<<<<< HEAD
	wstring fileName = decodeFromUtf8(entry.getFileEntry().tryGetRealPathName().str());
	if (fileName.empty())
=======
	std::wstring fileName = L"";
	if (entry != nullptr)
>>>>>>> 09ccbe42a1120f7185e91e13d9d2b8583217be7f
	{
		fileName = decodeFromUtf8(entry.getName().str());
	}
	else
	{
		fileName = FilePath(decodeFromUtf8(entry.getName().str())).getParentDirectory()
			.concatenate(FilePath(fileName).fileName()).wstr();
	}
	return fileName;
}

ParseLocation utility::getParseLocation(
	const clang::SourceLocation& sourceLocation,
	const clang::SourceManager& sourceManager,
	clang::Preprocessor* preprocessor,
	std::shared_ptr<CanonicalFilePathCache> canonicalFilePathCache)
{
	if (sourceLocation.isValid())
	{
		clang::SourceLocation loc = sourceLocation;
		if (sourceManager.isMacroBodyExpansion(sourceLocation))
		{
			loc = sourceManager.getExpansionLoc(sourceLocation);
			if (loc.isInvalid())
			{
				loc = sourceLocation;
			}
		}

		const clang::SourceLocation startLoc = sourceManager.getSpellingLoc(loc);
		const clang::FileID fileId = sourceManager.getFileID(startLoc);

		// find the start location
		const unsigned int startOffset = sourceManager.getFileOffset(startLoc);

		// General case -- find the end of the token starting at loc.
		if (preprocessor != nullptr)
		{
			const clang::SourceLocation endSloc = preprocessor->getLocForEndOfToken(startLoc);
			const unsigned int endOffset = sourceManager.getFileOffset(endSloc);

			return ParseLocation(
				canonicalFilePathCache->getFileSymbolId(fileId),
				sourceManager.getLineNumber(fileId, startOffset),
				sourceManager.getColumnNumber(fileId, startOffset),
				sourceManager.getLineNumber(fileId, endOffset),
				sourceManager.getColumnNumber(fileId, endOffset) - 1);
		}
		else
		{
			return ParseLocation(
				canonicalFilePathCache->getFileSymbolId(fileId),
				sourceManager.getLineNumber(fileId, startOffset),
				sourceManager.getColumnNumber(fileId, startOffset));
		}
	}

	return ParseLocation();
}

ParseLocation utility::getParseLocation(
	const clang::SourceRange& sourceRange,
	const clang::SourceManager& sourceManager,
	clang::Preprocessor* preprocessor,
	std::shared_ptr<CanonicalFilePathCache> canonicalFilePathCache)
{
	if (sourceRange.isValid())
	{
		clang::SourceRange range = sourceRange;
		clang::SourceLocation endLoc = preprocessor->getLocForEndOfToken(range.getEnd());

		if ((sourceManager.isMacroArgExpansion(range.getBegin()) ||
			 sourceManager.isMacroBodyExpansion(range.getBegin())) &&
			(sourceManager.isMacroArgExpansion(range.getEnd()) ||
			 sourceManager.isMacroBodyExpansion(range.getEnd())))
		{
			range = sourceManager.getExpansionRange(sourceRange).getAsRange();
			if (range.isValid())
			{
				endLoc = preprocessor->getLocForEndOfToken(range.getBegin());
			}
			else
			{
				range = sourceRange;
			}
		}

		const clang::SourceLocation beginLoc = range.getBegin();

		const clang::PresumedLoc presumedBegin = sourceManager.getPresumedLoc(beginLoc, false);
		const clang::PresumedLoc presumedEnd = sourceManager.getPresumedLoc(
			endLoc.isValid() ? endLoc : range.getEnd(), false);

		Id fileSymbolId = canonicalFilePathCache->getFileSymbolId(sourceManager.getFileID(beginLoc));
		if (!fileSymbolId)
		{
			fileSymbolId = canonicalFilePathCache->getFileSymbolId(
				utility::decodeFromUtf8(presumedBegin.getFilename()));
		}

		return ParseLocation(
			fileSymbolId,
			presumedBegin.getLine(),
			presumedBegin.getColumn(),
			presumedEnd.getLine(),
			presumedEnd.getColumn() - (endLoc.isValid() ? 1 : 0));
	}

	return ParseLocation();
}

PrintingPolicy utility::makePrintingPolicyForCPlusPlus()
{
	PrintingPolicy pp = PrintingPolicy(clang::LangOptions());
	// PrintingPolicy::adjustForCPlusPlus would set the same flags.
	pp.SuppressTagKeyword = true; // value "true": for a class A it prints "A" instead of "class A"
	pp.Bool = true;               // value "true": prints bool type as "bool" instead of "_Bool"

	return pp;
}
