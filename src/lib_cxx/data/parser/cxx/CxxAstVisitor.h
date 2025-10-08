#ifndef CXX_AST_VISITOR_H
#define CXX_AST_VISITOR_H

#include <memory>

#pragma warning(push)
#pragma warning(disable: 4702) // unreachable code
#include <clang/AST/RecursiveASTVisitor.h>
#pragma warning(pop)

#include "CxxAstVisitorComponentBraceRecorder.h"
#include "CxxAstVisitorComponentContext.h"
#include "CxxAstVisitorComponentDeclRefKind.h"
#include "CxxAstVisitorComponentImplicitCode.h"
#include "CxxAstVisitorComponentIndexer.h"
#include "CxxAstVisitorComponentTypeRefKind.h"

class CanonicalFilePathCache;
class ParserClient;
class FilePath;

struct IndexerStateInfo;
struct ParseLocation;

// methods are called in this order:
//	TraverseDecl()
//	`-	TraverseFunctionDecl()
// 		|-	WalkUpFromFunctionDecl()
// 		|	|-	WalkUpFromNamedDecl()
// 		|	|	|-	WalkUpFromDecl()
//	 	|	|	|	`-	VisitDecl()
// 		|	|	`-	VisitNamedDecl()
// 		|	`-	VisitFunctionDecl()
// 		`-	TraverseChildNodes()

// The "curiously recurring template pattern (CRTP)" doesn't need virtual functions. So the
// existing virtual functions exist only for 'CxxVerboseAstVisitor'.
class CxxAstVisitor: public clang::RecursiveASTVisitor<CxxAstVisitor>
{
public:
	CxxAstVisitor(
		clang::ASTContext* astContext,
		clang::Preprocessor* preprocessor,
		std::shared_ptr<ParserClient> client,
		std::shared_ptr<CanonicalFilePathCache> canonicalFilePathCache,
		std::shared_ptr<IndexerStateInfo> indexerStateInfo);
	virtual ~CxxAstVisitor() = default;

	template <typename T>
	T* getComponent();

	CanonicalFilePathCache* getCanonicalFilePathCache() const;

	// Indexing entry point
	void indexDecl(clang::Decl* d);

	// Visitor options
	virtual bool shouldVisitTemplateInstantiations() const;
	virtual bool shouldVisitImplicitCode() const;

	static bool shouldHandleTypeLoc(const clang::TypeLoc& tl);

	// Traversal methods. These specify how to traverse the AST and record context info.
	virtual bool TraverseDecl(clang::Decl* d);
	bool TraverseQualifiedTypeLoc(clang::QualifiedTypeLoc tl);
	virtual bool TraverseTypeLoc(clang::TypeLoc tl);
	bool TraverseType(clang::QualType t);
	virtual bool TraverseStmt(clang::Stmt* stmt);

	bool TraverseCXXRecordDecl(clang::CXXRecordDecl* d);
	bool traverseCXXBaseSpecifier(const clang::CXXBaseSpecifier& d);
	bool TraverseCXXMethodDecl(clang::CXXMethodDecl* d);
	bool TraverseTemplateTypeParmDecl(clang::TemplateTypeParmDecl* d);
	bool TraverseTemplateTemplateParmDecl(clang::TemplateTemplateParmDecl* d);
	bool TraverseNestedNameSpecifierLoc(clang::NestedNameSpecifierLoc loc);
	bool TraverseConstructorInitializer(clang::CXXCtorInitializer* init);
	bool TraverseCallExpr(clang::CallExpr* s);
	bool TraverseCXXMemberCallExpr(clang::CXXMemberCallExpr* s);
	bool TraverseCXXOperatorCallExpr(clang::CXXOperatorCallExpr* s);
	bool TraverseCXXConstructExpr(clang::CXXConstructExpr* s);
	bool TraverseCXXTemporaryObjectExpr(clang::CXXTemporaryObjectExpr* s);
	bool TraverseLambdaExpr(clang::LambdaExpr* s);
	bool TraverseFunctionDecl(clang::FunctionDecl* d);
	bool TraverseClassTemplateSpecializationDecl(clang::ClassTemplateSpecializationDecl* d);
	bool TraverseClassTemplatePartialSpecializationDecl(clang::ClassTemplatePartialSpecializationDecl* d);
	bool TraverseDeclRefExpr(clang::DeclRefExpr* s);
	bool TraverseCXXForRangeStmt(clang::CXXForRangeStmt* s);
	bool TraverseTemplateSpecializationTypeLoc(clang::TemplateSpecializationTypeLoc loc);
	bool TraverseUnresolvedLookupExpr(clang::UnresolvedLookupExpr* s);
	bool TraverseUnresolvedMemberExpr(clang::UnresolvedMemberExpr* S);
	bool TraverseTemplateArgumentLoc(const clang::TemplateArgumentLoc& loc);
	bool TraverseLambdaCapture(clang::LambdaExpr* lambdaExpr, const clang::LambdaCapture* capture, clang::Expr* Init);
	bool TraverseBinComma(clang::BinaryOperator* s);

	static bool TraverseDeclarationNameInfo(clang::DeclarationNameInfo NameInfo);

#define OPERATOR(NAME)                                                                             \
	bool TraverseBin##NAME##Assign(clang::CompoundAssignOperator* s)                       \
	{                                                                                              \
		return TraverseAssignCommon(s);                                                            \
	}
	OPERATOR(Mul)
	OPERATOR(Div)
	OPERATOR(Rem)
	OPERATOR(Add)
	OPERATOR(Sub)
	OPERATOR(Shl)
	OPERATOR(Shr)
	OPERATOR(And)
	OPERATOR(Or)
	OPERATOR(Xor)
#undef OPERATOR


	void traverseDeclContextHelper(clang::DeclContext* d);
	bool TraverseCallCommon(clang::CallExpr* s);
	bool TraverseAssignCommon(clang::BinaryOperator* s);

	// Visitor methods. These actually record stuff and store it in the database.
	bool VisitCastExpr(clang::CastExpr* s);
	bool VisitUnaryAddrOf(clang::UnaryOperator* s);
	bool VisitUnaryDeref(clang::UnaryOperator* s);
	bool VisitDeclStmt(clang::DeclStmt* s);
	bool VisitReturnStmt(clang::ReturnStmt* s);
	bool VisitCompoundStmt(clang::CompoundStmt* s);
	bool VisitInitListExpr(clang::InitListExpr* s);


	bool VisitTagDecl(clang::TagDecl* d);
	bool VisitClassTemplateDecl(clang::ClassTemplateDecl *d);
	bool VisitClassTemplateSpecializationDecl(clang::ClassTemplateSpecializationDecl* d);
	bool VisitFunctionDecl(clang::FunctionDecl* d);
	bool VisitFunctionTemplateDecl(clang::FunctionTemplateDecl *d);
	bool VisitCXXMethodDecl(clang::CXXMethodDecl* d);
	bool VisitVarDecl(clang::VarDecl* d);
	bool VisitVarTemplateSpecializationDecl(clang::VarTemplateSpecializationDecl* d);
	bool VisitFieldDecl(clang::FieldDecl* d);
	bool VisitTypedefDecl(clang::TypedefDecl* d);
	bool VisitTypeAliasDecl(clang::TypeAliasDecl* d);
	bool VisitNamespaceDecl(clang::NamespaceDecl* d);
	bool VisitNamespaceAliasDecl(clang::NamespaceAliasDecl* d);
	bool VisitEnumConstantDecl(clang::EnumConstantDecl* d);
	bool VisitUsingDirectiveDecl(clang::UsingDirectiveDecl* d);
	bool VisitUsingDecl(clang::UsingDecl* d);
	bool VisitNonTypeTemplateParmDecl(clang::NonTypeTemplateParmDecl* d);
	bool VisitTemplateTypeParmDecl(clang::TemplateTypeParmDecl* d);
	bool VisitTemplateTemplateParmDecl(clang::TemplateTemplateParmDecl* d);
	bool VisitConceptDecl(clang::ConceptDecl *d);
	bool VisitConceptSpecializationExpr(clang::ConceptSpecializationExpr *d);
	static bool VisitTranslationUnitDecl(clang::TranslationUnitDecl* d);

	bool VisitTypeLoc(clang::TypeLoc tl);

	bool VisitDeclRefExpr(clang::DeclRefExpr* s);
	bool VisitMemberExpr(clang::MemberExpr* s);
	bool VisitCXXDependentScopeMemberExpr(clang::CXXDependentScopeMemberExpr* s);
	bool VisitCXXConstructExpr(clang::CXXConstructExpr* s);
	bool VisitCXXDeleteExpr(clang::CXXDeleteExpr* s);
	bool VisitLambdaExpr(clang::LambdaExpr* s);
	bool VisitMSAsmStmt(clang::MSAsmStmt* s);
	bool VisitConstructorInitializer(clang::CXXCtorInitializer* init);

	ParseLocation getParseLocationOfTagDeclBody(clang::TagDecl* decl) const;
	ParseLocation getParseLocationOfFunctionBody(const clang::FunctionDecl* decl) const;
	ParseLocation getParseLocation(const clang::SourceLocation& loc) const;
	ParseLocation getParseLocation(const clang::SourceRange& sourceRange) const;

	bool shouldVisitStmt(const clang::Stmt* stmt) const;
	bool shouldVisitDecl(const clang::Decl* decl) const;
	bool shouldVisitReference(const clang::SourceLocation& referenceLocation) const;

	bool isLocatedInProjectFile(clang::SourceLocation loc) const;

protected:
	typedef clang::RecursiveASTVisitor<CxxAstVisitor> Base;

	clang::ASTContext* m_astContext;
	clang::Preprocessor* m_preprocessor;
	std::shared_ptr<ParserClient> m_client;
	std::shared_ptr<IndexerStateInfo> m_indexerStateInfo;
	std::shared_ptr<CanonicalFilePathCache> m_canonicalFilePathCache;

	CxxAstVisitorComponentContext m_contextComponent;
	CxxAstVisitorComponentDeclRefKind m_declRefKindComponent;
	CxxAstVisitorComponentTypeRefKind m_typeRefKindComponent;
	CxxAstVisitorComponentImplicitCode m_implicitCodeComponent;
	CxxAstVisitorComponentIndexer m_indexerComponent;
	CxxAstVisitorComponentBraceRecorder m_braceRecorderComponent;
};

template <>
CxxAstVisitorComponentContext* CxxAstVisitor::getComponent();

template <>
CxxAstVisitorComponentTypeRefKind* CxxAstVisitor::getComponent();

template <>
CxxAstVisitorComponentDeclRefKind* CxxAstVisitor::getComponent();

template <>
CxxAstVisitorComponentIndexer* CxxAstVisitor::getComponent();

#endif	  // CXX_AST_VISITOR_H
