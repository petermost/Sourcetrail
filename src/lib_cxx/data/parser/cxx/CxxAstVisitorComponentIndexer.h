#ifndef CXX_AST_VISITOR_COMPONENT_INDEXER_H
#define CXX_AST_VISITOR_COMPONENT_INDEXER_H

#include <map>

#include "CxxAstVisitorComponent.h"
#include "ParseLocation.h"
#include "ReferenceKind.h"
#include "SymbolKind.h"

class CxxContext;
class ParserClient;
class NameHierarchy;

// This CxxAstVisitorComponent is responsible for recording all symbols and relations throughout the
// visited AST.
class CxxAstVisitorComponentIndexer: public CxxAstVisitorComponent
{
public:
	CxxAstVisitorComponentIndexer(
		CxxAstVisitor* astVisitor, clang::ASTContext* astContext, std::shared_ptr<ParserClient> client);

	void beginTraverseNestedNameSpecifierLoc(const clang::NestedNameSpecifierLoc& loc);
	void beginTraverseTemplateArgumentLoc(const clang::TemplateArgumentLoc& loc);
	void beginTraverseLambdaCapture(clang::LambdaExpr* lambdaExpr, const clang::LambdaCapture* capture);

	void visitCastExpr(clang::CastExpr *d);
	void visitCXXFunctionalCastExpr(clang::CXXFunctionalCastExpr *d);
	void visitTagDecl(clang::TagDecl* d);
	void visitClassTemplateDecl(clang::ClassTemplateDecl *d);
	void visitClassTemplateSpecializationDecl(clang::ClassTemplateSpecializationDecl* d);
	void visitVarDecl(clang::VarDecl* d);
	void visitDecompositionDecl(clang::DecompositionDecl *d);
	void visitVarTemplateSpecializationDecl(clang::VarTemplateSpecializationDecl* d);
	void visitFieldDecl(clang::FieldDecl* d);
	void visitFunctionDecl(clang::FunctionDecl* d);
	void visitFunctionTemplateDecl(clang::FunctionTemplateDecl *d);
	void visitCXXMethodDecl(clang::CXXMethodDecl* d);
	void visitEnumConstantDecl(clang::EnumConstantDecl* d);
	void visitNamespaceDecl(clang::NamespaceDecl* d);
	void visitNamespaceAliasDecl(clang::NamespaceAliasDecl* d);
	void visitTypedefDecl(clang::TypedefDecl* d);
	void visitTypeAliasDecl(clang::TypeAliasDecl* d);
	void visitUsingDirectiveDecl(clang::UsingDirectiveDecl* d);
	void visitUsingDecl(clang::UsingDecl* d);
	void visitNonTypeTemplateParmDecl(clang::NonTypeTemplateParmDecl* d);
	void visitTemplateTypeParmDecl(clang::TemplateTypeParmDecl* d);
	void visitTemplateTemplateParmDecl(clang::TemplateTemplateParmDecl* d);
	void visitConceptDecl(clang::ConceptDecl *d);
	void visitConceptSpecializationExpr(clang::ConceptSpecializationExpr *d);
	void visitConceptReference(clang::ConceptReference *d);
	void visitTypeLoc(clang::TypeLoc tl);

	void visitDeclRefExpr(clang::DeclRefExpr* s);
	void visitMemberExpr(clang::MemberExpr* s);
	void visitCXXConstructExpr(clang::CXXConstructExpr* s);
	void visitCXXDeleteExpr(clang::CXXDeleteExpr* s);
	void visitLambdaExpr(clang::LambdaExpr* s);

	void visitConstructorInitializer(clang::CXXCtorInitializer* init);

	// Module support:

	void visitTranslationUnitDecl(clang::TranslationUnitDecl *d);
	void visitExportDecl(clang::ExportDecl *d);
	void visitImportDecl(clang::ImportDecl *d);

private:
	void recordTemplateMemberSpecialization( const clang::MemberSpecializationInfo* memberSpecializationInfo, Id contextId,
		const ParseLocation& location, SymbolKind symbolKind);

	void recordTemplateParameterConceptReferences(const clang::TemplateDecl *templateDecl);
	template <typename T> void recordConceptReference(const T *d);
	void recordNamedConceptReference(const clang::ConceptReference *conceptReference);

	void recordDeducedType(const clang::DeducedType *autoType, const Id contextSymbolId, const ParseLocation &keywordLocation);
	void recordDeducedQualType(const clang::QualType deducedQualType, const Id contextSymbolId, const ParseLocation &keywordLocation);

	void recordNonTrivialDestructorCalls(const clang::FunctionDecl *d);

	ParseLocation getSignatureLocation(clang::FunctionDecl* d);
	ParseLocation getParseLocationOfTagDeclBody(clang::TagDecl* decl) const;
	ParseLocation getParseLocationOfFunctionBody(const clang::FunctionDecl* decl) const;
	ParseLocation getParseLocation(const clang::SourceLocation& loc) const;
	ParseLocation getParseLocation(const clang::SourceRange& sourceRange) const;

	std::string getLocalSymbolName(const clang::SourceLocation& loc) const;

	ReferenceKind consumeDeclRefContextKind();

	Id getOrCreateSymbolId(const clang::NamedDecl* decl);
	Id getOrCreateSymbolId(const clang::Type* type);
	Id getOrCreateSymbolId(const CxxContext* context);
	Id getOrCreateSymbolId(const CxxContext* context, const NameHierarchy& fallback);

	template <typename NameResolver, typename T>
	NameHierarchy makeNameHierarchy(const T *t);

	clang::ASTContext* m_astContext;
	std::shared_ptr<ParserClient> m_client;

	std::map<const clang::NamedDecl*, Id> m_declSymbolIds;
	std::map<const clang::Type*, Id> m_typeSymbolIds;
};

#endif	  // CXX_AST_VISITOR_COMPONENT_INDEXER_H
