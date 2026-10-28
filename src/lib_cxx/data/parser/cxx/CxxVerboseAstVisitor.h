
#ifndef CXX_VERBOSE_AST_VISITOR_H
#define CXX_VERBOSE_AST_VISITOR_H

#include <clang/AST/TypeLoc.h>

#include "CxxAstVisitor.h"

class CanonicalFilePathCache;
class ParserClient;

class CxxVerboseAstVisitor: public CxxAstVisitor
{
public:
	CxxVerboseAstVisitor(
		clang::ASTContext* context,
		clang::Preprocessor* preprocessor,
		std::shared_ptr<ParserClient> client,
		std::shared_ptr<CanonicalFilePathCache> canonicalFilePathCache,
		std::shared_ptr<IndexerStateInfo> indexerStateInfo);

private:
	bool TraverseDecl(clang::Decl* d) override;
	bool TraverseStmt(clang::Stmt* stmt) override;
	bool TraverseTypeLoc(clang::TypeLoc tl, bool traverseQualifier = true) override;

	std::string getIndentString() const;
	static std::string obfuscateName(const std::string& name);

	static std::string typeLocClassToString(clang::TypeLoc tl) 
	{
		switch (tl.getTypeLocClass())
		{
#define STRINGIFY(X) #X
#define ABSTRACT_TYPE(Class, Base)
#define TYPE(Class, Base)                                                                          \
case clang::TypeLoc::Class:                                                                        \
	return STRINGIFY(Class);
#include <clang/AST/TypeLoc.h>
		case clang::TypeLoc::TypeLocClass::Qualified:
			return "Qualified";
        default:
            return "";
		}
	}

	FilePath m_currentFilePath;
	unsigned int m_indentation = 0;
};

#endif	  // CXX_VERBOSE_AST_VISITOR_H
