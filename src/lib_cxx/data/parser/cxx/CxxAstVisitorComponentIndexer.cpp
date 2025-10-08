#include "CxxAstVisitorComponentIndexer.h"

#include <clang/AST/ASTContext.h>
#include <clang/Analysis/CFG.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Preprocessor.h>

#include "CanonicalFilePathCache.h"
#include "CxxAstVisitor.h"
#include "CxxAstVisitorComponentContext.h"
#include "CxxAstVisitorComponentDeclRefKind.h"
#include "CxxAstVisitorComponentTypeRefKind.h"
#include "CxxDeclNameResolver.h"
#include "CxxFunctionDeclName.h"
#include "CxxTypeNameResolver.h"
#include "ParserClient.h"
#include "utilityClang.h"

using namespace std;
using namespace clang;

CxxAstVisitorComponentIndexer::CxxAstVisitorComponentIndexer(
	CxxAstVisitor* astVisitor, clang::ASTContext* astContext, std::shared_ptr<ParserClient> client)
	: CxxAstVisitorComponent(astVisitor), m_astContext(astContext), m_client(client)
{
}

void CxxAstVisitorComponentIndexer::beginTraverseNestedNameSpecifierLoc(
	const clang::NestedNameSpecifierLoc& loc)
{
	if (!getAstVisitor()->shouldVisitReference(loc.getBeginLoc()))
	{
		return;
	}

	switch (loc.getNestedNameSpecifier()->getKind())
	{
	case clang::NestedNameSpecifier::Identifier:
		break;
	case clang::NestedNameSpecifier::Namespace:
	{
		Id symbolId = getOrCreateSymbolId(loc.getNestedNameSpecifier()->getAsNamespace());
		m_client->recordSymbolKind(symbolId, SymbolKind::NAMESPACE);
		m_client->recordLocation(
			symbolId, getParseLocation(loc.getLocalBeginLoc()), ParseLocationType::QUALIFIER);
	}
	break;
	case clang::NestedNameSpecifier::NamespaceAlias:
	{
		Id symbolId = getOrCreateSymbolId(loc.getNestedNameSpecifier()->getAsNamespaceAlias());
		m_client->recordSymbolKind(symbolId, SymbolKind::NAMESPACE);

		symbolId = getOrCreateSymbolId(
			loc.getNestedNameSpecifier()->getAsNamespaceAlias()->getAliasedNamespace());
		m_client->recordSymbolKind(symbolId, SymbolKind::NAMESPACE);
	}
	break;
	case clang::NestedNameSpecifier::Global:
	case clang::NestedNameSpecifier::Super:
		break;
	case clang::NestedNameSpecifier::TypeSpec:
	case clang::NestedNameSpecifier::TypeSpecWithTemplate:
		if (const clang::CXXRecordDecl* recordDecl = loc.getNestedNameSpecifier()->getAsRecordDecl())
		{
			SymbolKind symbolKind = SymbolKind::UNDEFINED;
			if (recordDecl->isClass())
			{
				symbolKind = SymbolKind::CLASS;
			}
			else if (recordDecl->isStruct())
			{
				symbolKind = SymbolKind::STRUCT;
			}
			else if (recordDecl->isUnion())
			{
				symbolKind = SymbolKind::UNION;
			}

			if (symbolKind != SymbolKind::UNDEFINED)
			{
				const Id symbolId = getOrCreateSymbolId(recordDecl);
				m_client->recordSymbolKind(symbolId, symbolKind);
				m_client->recordLocation(
					symbolId, getParseLocation(loc.getLocalBeginLoc()), ParseLocationType::QUALIFIER);
			}
		}
		else if (const clang::Type* type = loc.getNestedNameSpecifier()->getAsType())
		{
			const ParseLocation parseLocation = getParseLocation(loc.getLocalBeginLoc());

			if (const clang::TemplateTypeParmType* tpt =
					clang::dyn_cast_or_null<clang::TemplateTypeParmType>(type))
			{
				clang::TemplateTypeParmDecl* d = tpt->getDecl();
				if (d)
				{
					m_client->recordLocalSymbol(getLocalSymbolName(d->getLocation()), parseLocation);
				}
			}
			else
			{
				const Id symbolId = getOrCreateSymbolId(type);
				m_client->recordLocation(symbolId, parseLocation, ParseLocationType::QUALIFIER);
			}
		}
	}
}

void CxxAstVisitorComponentIndexer::beginTraverseTemplateArgumentLoc(
	const clang::TemplateArgumentLoc& loc)
{
	if (getAstVisitor()->shouldVisitReference(loc.getLocation()))
	{
		if (loc.getArgument().getKind() == clang::TemplateArgument::Template)
		{
			// TODO: maybe move this to VisitTemplateName

			const clang::TemplateName templateTemplateArgumentName = loc.getArgument().getAsTemplate();

			const ParseLocation parseLocation = getParseLocation(loc.getLocation());
			if (templateTemplateArgumentName.isDependent())
			{
				clang::SourceLocation declLocation;
				if (templateTemplateArgumentName.getAsTemplateDecl())
				{
					declLocation = templateTemplateArgumentName.getAsTemplateDecl()->getLocation();
				}
				else
				{
					declLocation = loc.getLocation();
				}
				m_client->recordLocalSymbol(getLocalSymbolName(declLocation), parseLocation);
			}
			else
			{
				const Id symbolId = getOrCreateSymbolId(
					templateTemplateArgumentName.getAsTemplateDecl());

				m_client->recordReference(
					REFERENCE_TYPE_USAGE,
					symbolId,
					getOrCreateSymbolId(
						getAstVisitor()->getComponent<CxxAstVisitorComponentContext>()->getContext()),
					parseLocation);

				{
					if (const clang::NamedDecl* namedContextDecl =
							getAstVisitor()
								->getComponent<CxxAstVisitorComponentContext>()
								->getTopmostContextDecl(1))
					{
						m_client->recordReference(
							REFERENCE_TYPE_USAGE,
							symbolId,
							getOrCreateSymbolId(namedContextDecl),	  // we use the closest named decl
																	  // here (e.g. function decl)
							parseLocation);
					}
				}
			}
		}
	}
}

void CxxAstVisitorComponentIndexer::beginTraverseLambdaCapture(
	clang::LambdaExpr* lambdaExpr, const clang::LambdaCapture* capture)
{
	if ((!lambdaExpr->isInitCapture(capture)) && (capture->capturesVariable()))
	{
		clang::ValueDecl* d = capture->getCapturedVar();
		if (utility::isLocalVariable(d) || utility::isParameter(d))
		{
			if (!d->getNameAsString().empty())	  // don't record anonymous parameters
			{
				m_client->recordLocalSymbol(
					getLocalSymbolName(d->getLocation()), getParseLocation(capture->getLocation()));
			}
		}
	}
}

void CxxAstVisitorComponentIndexer::visitCastExpr(clang::CastExpr *d)
{
	if (getAstVisitor()->shouldVisitStmt(d))
	{
		if (d->getCastKind() == clang::CK_UserDefinedConversion)
		{
			Id referencedSymbolId = getOrCreateSymbolId(d->getConversionFunction());
			Id contextSymbolId = getOrCreateSymbolId(getAstVisitor()->getComponent<CxxAstVisitorComponentContext>()->getContext());
			ParseLocation location = getParseLocation(d->getSourceRange());

			m_client->recordReference(REFERENCE_CALL, referencedSymbolId, contextSymbolId, location);
		}
	}
}

void CxxAstVisitorComponentIndexer::visitTagDecl(clang::TagDecl* d)
{
	if (getAstVisitor()->shouldVisitDecl(d))
	{
		DefinitionKind definitionKind = DefinitionKind::NONE;
		if (d->isThisDeclarationADefinition())
		{
			definitionKind = utility::getDefinitionKind(d);
		}

		const SymbolKind symbolKind = utility::convertTagKind(d->getTagKind());
		const ParseLocation location = getParseLocation(d->getLocation());

		Id symbolId = getOrCreateSymbolId(d);
		m_client->recordSymbolKind(symbolId, symbolKind);
		m_client->recordLocation(symbolId, location, ParseLocationType::TOKEN);
		m_client->recordLocation(
			symbolId, getParseLocationOfTagDeclBody(d), ParseLocationType::SCOPE);
		m_client->recordAccessKind(symbolId, utility::convertAccessSpecifier(d->getAccess()));
		m_client->recordDefinitionKind(symbolId, definitionKind);

		if (clang::EnumDecl* enumDecl = clang::dyn_cast_or_null<clang::EnumDecl>(d))
		{
			recordTemplateMemberSpecialization(
				enumDecl->getMemberSpecializationInfo(), symbolId, location, symbolKind);
		}

		if (clang::CXXRecordDecl* recordDecl = clang::dyn_cast_or_null<clang::CXXRecordDecl>(d))
		{
			recordTemplateMemberSpecialization(
				recordDecl->getMemberSpecializationInfo(), symbolId, location, symbolKind);
		}
	}
}

void CxxAstVisitorComponentIndexer::visitClassTemplateDecl(clang::ClassTemplateDecl *d)
{
	if (getAstVisitor()->shouldVisitDecl(d))
	{
		recordTemplateParameterConceptReferences(d);
	}
}

void CxxAstVisitorComponentIndexer::visitClassTemplateSpecializationDecl(
	clang::ClassTemplateSpecializationDecl* d)
{
	if (getAstVisitor()->shouldVisitDecl(d))
	{
		clang::CXXRecordDecl* specializedFromDecl = nullptr;

		llvm::PointerUnion<clang::ClassTemplateDecl*, clang::ClassTemplatePartialSpecializationDecl*> pu =
			d->getSpecializedTemplateOrPartial();
		if (clang::isa<clang::ClassTemplateDecl*>(pu))
		{
			specializedFromDecl = clang::cast<clang::ClassTemplateDecl*>(pu)->getTemplatedDecl();
		}
		else if (clang::isa<clang::ClassTemplatePartialSpecializationDecl*>(pu))
		{
			specializedFromDecl = clang::cast<clang::ClassTemplatePartialSpecializationDecl*>(pu);
		}

		m_client->recordReference(
			REFERENCE_TEMPLATE_SPECIALIZATION,
			getOrCreateSymbolId(specializedFromDecl),
			getOrCreateSymbolId(d),
			getParseLocation(d->getLocation()));
	}
}

void CxxAstVisitorComponentIndexer::visitVarDecl(clang::VarDecl* d)
{
	if (getAstVisitor()->shouldVisitDecl(d))
	{
		// string _varName = d->getNameAsString();
		// string _typeName = d->getType().getAsString();

		// Record auto/deduced types:
		if (const DeducedType *deducedVariableType = d->getType()->getContainedDeducedType())
		{
			for (TypeLoc typeLoc = d->getTypeSourceInfo()->getTypeLoc(); typeLoc; typeLoc = typeLoc.getNextTypeLoc())
			{
				if (const AutoTypeLoc &autoTypeLoc = typeLoc.getAs<AutoTypeLoc>())
				{
					if (const AutoType *autoVariableType = dyn_cast<AutoType>(autoTypeLoc.getTypePtr()))
					{
						if (const ConceptDecl *conceptDecl = autoVariableType->getTypeConstraintConcept())
						{
							// Record concept name location:
							const ParseLocation conceptNameLocation = getParseLocation(autoTypeLoc.getConceptNameLoc());
							m_client->recordReference(REFERENCE_USAGE, getOrCreateSymbolId(conceptDecl), getOrCreateSymbolId(d), conceptNameLocation);

							// Record 'auto' location:
							const ParseLocation autoKeywordLocation = getParseLocation(autoTypeLoc.getNameLoc());
							recordDeducedType(deducedVariableType, d, autoKeywordLocation);
						}
						else
						{
							// Record 'auto' or 'decltype(auto)' location:
							if (autoVariableType->isDecltypeAuto())
							{
								const ParseLocation decltypeAutoKeywordLocation = getParseLocation(autoTypeLoc.getSourceRange());
								recordDeducedType(deducedVariableType, d, decltypeAutoKeywordLocation);
							}
							else
							{
								const ParseLocation autoKeywordLocation = getParseLocation(autoTypeLoc.getBeginLoc());
								recordDeducedType(deducedVariableType, d, autoKeywordLocation);
							}
						}
					}
				}
			}
		}
		if (utility::isLocalVariable(d) || utility::isParameter(d))
		{
			if (!d->getNameAsString().empty())	  // don't record anonymous parameters
			{
				m_client->recordLocalSymbol(getLocalSymbolName(d->getLocation()), getParseLocation(d->getLocation()));
			}
		}
		else
		{
			const SymbolKind symbolKind = utility::getSymbolKind(d);
			const ParseLocation location = getParseLocation(d->getLocation());

			Id symbolId = getOrCreateSymbolId(d);
			m_client->recordSymbolKind(symbolId, symbolKind);
			m_client->recordLocation(symbolId, location, ParseLocationType::TOKEN);
			m_client->recordAccessKind(symbolId, utility::convertAccessSpecifier(d->getAccess()));
			m_client->recordDefinitionKind(symbolId, utility::getDefinitionKind(d));

			recordTemplateMemberSpecialization(d->getMemberSpecializationInfo(), symbolId, location, symbolKind);
		}
	}
}

void CxxAstVisitorComponentIndexer::visitVarTemplateSpecializationDecl(
	clang::VarTemplateSpecializationDecl* d)
{
	if (getAstVisitor()->shouldVisitDecl(d))
	{
		clang::NamedDecl* specializedFromDecl = nullptr;

		// todo: use context and childcontext!!
		llvm::PointerUnion<clang::VarTemplateDecl*, clang::VarTemplatePartialSpecializationDecl*> pu =
			d->getSpecializedTemplateOrPartial();
		if (clang::isa<clang::VarTemplateDecl*>(pu))
		{
			specializedFromDecl = clang::cast<clang::VarTemplateDecl*>(pu);
		}
		else if (clang::isa<clang::VarTemplatePartialSpecializationDecl*>(pu))
		{
			specializedFromDecl = clang::cast<clang::VarTemplatePartialSpecializationDecl*>(pu);
		}

		m_client->recordReference(
			REFERENCE_TEMPLATE_SPECIALIZATION,
			getOrCreateSymbolId(specializedFromDecl),
			getOrCreateSymbolId(d),
			getParseLocation(d->getLocation()));
	}
}

void CxxAstVisitorComponentIndexer::visitFieldDecl(clang::FieldDecl* d)
{
	if (getAstVisitor()->shouldVisitDecl(d))
	{
		if (clang::isa<clang::ObjCIvarDecl>(d))
		{
			return;
		}

		const ParseLocation location = getParseLocation(d->getLocation());

		Id fieldId = getOrCreateSymbolId(d);
		m_client->recordSymbolKind(fieldId, SymbolKind::FIELD);
		m_client->recordLocation(fieldId, location, ParseLocationType::TOKEN);
		m_client->recordAccessKind(fieldId, utility::convertAccessSpecifier(d->getAccess()));
		m_client->recordDefinitionKind(fieldId, utility::getDefinitionKind(d));

		if (clang::CXXRecordDecl* declaringRecordDecl =
				clang::dyn_cast_or_null<clang::CXXRecordDecl>(d->getParent()))
		{
			if (clang::CXXRecordDecl* declaringRecordTemplateDecl =
					declaringRecordDecl->getTemplateInstantiationPattern())
			{
				for (clang::FieldDecl* templateFieldDecl: declaringRecordTemplateDecl->fields())
				{
					if (d->getDeclName().isIdentifier() &&
						templateFieldDecl->getDeclName().isIdentifier() &&
						d->getName() == templateFieldDecl->getName())
					{
						Id templateFieldId = getOrCreateSymbolId(templateFieldDecl);
						m_client->recordSymbolKind(templateFieldId, SymbolKind::FIELD);
						m_client->recordReference(
							REFERENCE_TEMPLATE_SPECIALIZATION, templateFieldId, fieldId, location);
						break;
					}
				}
			}
		}
	}
}

void CxxAstVisitorComponentIndexer::visitFunctionDecl(clang::FunctionDecl* d)
{
	if (getAstVisitor()->shouldVisitDecl(d))
	{
		Id symbolId = getOrCreateSymbolId(d);
		m_client->recordSymbolKind(symbolId, clang::isa<clang::CXXMethodDecl>(d) ? SymbolKind::METHOD : SymbolKind::FUNCTION);
		m_client->recordLocation(symbolId, getParseLocation(d->getNameInfo().getSourceRange()), ParseLocationType::TOKEN);
		m_client->recordLocation(symbolId, getParseLocationOfFunctionBody(d), ParseLocationType::SCOPE);
		m_client->recordAccessKind(symbolId, utility::convertAccessSpecifier(d->getAccess()));
		m_client->recordDefinitionKind(symbolId, utility::getDefinitionKind(d));

		if (d->isFirstDecl())
		{
			m_client->recordLocation(symbolId, getSignatureLocation(d), ParseLocationType::SIGNATURE);
		}

		if (d->isFunctionTemplateSpecialization())
		{
			if (clang::isa<clang::ClassTemplateSpecializationDecl>(d->getParent()) &&
				!clang::isa<clang::ClassTemplatePartialSpecializationDecl>(d->getParent()) &&
				!clang::dyn_cast<clang::ClassTemplateSpecializationDecl>(d->getParent())->isExplicitSpecialization())
			{
				// record edge from Foo<int>::bar<float>() to Foo<T>::bar<U>() instead of recording
				// an edge from Foo<int>::bar<float>() to Foo<int>::bar<U>() because there is not
				// "written" code for Foo<int>::bar<U>() if Foo<int> is an implicit template
				// specialization.
				if (clang::CXXRecordDecl *declaringRecordDecl = clang::dyn_cast_or_null<clang::CXXRecordDecl>(d->getParent()))
				{
					if (clang::CXXRecordDecl *declaringRecordTemplateDecl = declaringRecordDecl->getTemplateInstantiationPattern())
					{
						for (clang::Decl *templateMethodDecl : declaringRecordTemplateDecl->decls())
						{
							if (clang::FunctionTemplateDecl *functionTemplateDecl = clang::dyn_cast_or_null<clang::FunctionTemplateDecl>(templateMethodDecl))
							{
								if (d->getDeclName().isIdentifier() && functionTemplateDecl->getDeclName().isIdentifier() &&
									d->getName() == functionTemplateDecl->getName())
								{
									const Id templateMethodId = getOrCreateSymbolId(functionTemplateDecl);
									m_client->recordSymbolKind(templateMethodId, SymbolKind::METHOD);
									m_client->recordReference(REFERENCE_TEMPLATE_SPECIALIZATION, templateMethodId, symbolId,
										getParseLocation(d->getLocation()));
									break;
								}
							}
						}
					}
				}
			}
			else
			{
				// record edge from foo<int>() to foo<T>()
				if (FunctionTemplateDecl *primaryTemplate = d->getPrimaryTemplate())
				{
					Id templateId = getOrCreateSymbolId(primaryTemplate->getTemplatedDecl());
					m_client->recordSymbolKind(templateId, SymbolKind::FUNCTION);
					m_client->recordReference(REFERENCE_TEMPLATE_SPECIALIZATION, templateId, symbolId, getParseLocation(d->getLocation()));
				}
			}
		}

		// Record deduced return type:

		if (const DeducedType *deducedReturnType = d->getReturnType()->getContainedDeducedType())
		{
			const SourceRange returnTypeSourceRange = d->getReturnTypeSourceRange();

			if (const AutoType *autoReturnType = dyn_cast<AutoType>(deducedReturnType))
			{
				if (const ConceptDecl *conceptDecl = autoReturnType->getTypeConstraintConcept())
				{
					// Record the concept reference:
					const ParseLocation conceptNameLocation = getParseLocation(returnTypeSourceRange.getBegin());
					m_client->recordReference(REFERENCE_USAGE, getOrCreateSymbolId(conceptDecl), getOrCreateSymbolId(d), conceptNameLocation);

					// Record the auto type:
					const ParseLocation autoKeywordLocation = getParseLocation(returnTypeSourceRange.getEnd());
					recordDeducedType(deducedReturnType, d, autoKeywordLocation);
				}
				else
				{
					// Record the auto/deduced return type:
					const ParseLocation autoOrDecltypeKeywordLocation = getParseLocation(returnTypeSourceRange);
					recordDeducedType(deducedReturnType, d, autoOrDecltypeKeywordLocation);
				}
			}
		}

		recordNonTrivialDestructorCalls(d);
	}
}

void CxxAstVisitorComponentIndexer::visitFunctionTemplateDecl(FunctionTemplateDecl *d)
{
	if (getAstVisitor()->shouldVisitDecl(d))
	{
		recordTemplateParameterConceptReferences(d);
	}
}

void CxxAstVisitorComponentIndexer::recordTemplateParameterConceptReferences(const TemplateDecl *templateDecl)
{
	if (const TemplateParameterList *templateParameters = templateDecl->getTemplateParameters())
	{
		for (const NamedDecl *namedDecl : *templateParameters)
		{
			if (const TemplateTypeParmDecl *templateTypeParmDecl = dyn_cast<TemplateTypeParmDecl>(namedDecl))
			{
				if (const TypeConstraint *typeConstraint = templateTypeParmDecl->getTypeConstraint())
				{
					recordConceptReference(typeConstraint);
				}
			}
		}
	}
}

template <typename T>
void CxxAstVisitorComponentIndexer::recordConceptReference(const T *d)
{
	if (const ConceptReference *conceptReference = d->getConceptReference())
	{
		if (const ConceptDecl *conceptDecl = conceptReference->getNamedConcept())
		{
			const Id conceptDeclId = getOrCreateSymbolId(conceptDecl);
			const Id contextSymbolId = getOrCreateSymbolId(getAstVisitor()->getComponent<CxxAstVisitorComponentContext>()->getContext());
			const ParseLocation conceptNameLocation = getParseLocation(conceptReference->getLocation());

			m_client->recordReference(REFERENCE_USAGE, conceptDeclId, contextSymbolId, conceptNameLocation);
		}
	}
}

void CxxAstVisitorComponentIndexer::recordDeducedType(const DeducedType *containedDeducedType, const NamedDecl *d, const ParseLocation &location)
{
	if (QualType deducedType = containedDeducedType->getDeducedType(); !deducedType.isNull())
	{
		// Record the deduced type location:
		Id deducedTypeId = getOrCreateSymbolId(deducedType.getTypePtr());

		m_client->recordDefinitionKind(deducedTypeId, DefinitionKind::EXPLICIT);

		// Record a reference to the type declaration:
		m_client->recordReference(REFERENCE_TYPE_USAGE, deducedTypeId, getOrCreateSymbolId(d), location);
	}
}

void CxxAstVisitorComponentIndexer::recordNonTrivialDestructorCalls(const FunctionDecl *functionDecl)
{
	auto recordDestructorCall = [this](const FunctionDecl *functionDecl, const CXXDestructorDecl *destructorDecl)
	{
		Id referencedSymbolId = getOrCreateSymbolId(destructorDecl);
		Id contextSymbolId = getOrCreateSymbolId(getAstVisitor()->getComponent<CxxAstVisitorComponentContext>()->getContext());
		// functionDecl->getLocation: The function name
		// functionDecl->getBeginLoc: Begin of function
		// functionDecl->getEndLoc: End of function
		// functionDecl->getSourceRange: The complete function
		// functionDecl->getDefaultLoc: No location but 'call' edge
		// destructorDecl->getSourceRange: The destructor itself

		ParseLocation parseLocation = getParseLocation(functionDecl->getEndLoc());
		m_client->recordReference(REFERENCE_CALL, referencedSymbolId, contextSymbolId, parseLocation);
	};

	// Adapted from:
	// "How to get information about call to destructors in Clang LibTooling?"
	// https://stackoverflow.com/questions/59610156/how-to-get-information-about-call-to-destructors-in-clang-libtooling

	if (functionDecl->isThisDeclarationADefinition())
	{
		CFG::BuildOptions buildOptions;
		buildOptions.AddImplicitDtors = true;
		buildOptions.AddTemporaryDtors = true;

		if (unique_ptr<CFG> cfg = CFG::buildCFG(functionDecl, functionDecl->getBody(), m_astContext, buildOptions))
		{
			for (CFGBlock *block : cfg->const_nodes())
			{
				for (auto ref : block->refs())
				{
					// It should not be necessary to special-case 'CFGBaseDtor'. But 'CFGImplicitDtor::getDestructorDecl' 
					// is simply missing the implementation of that case. See 'CFGImplicitDtor::getDestructorDecl()':
					// https://github.com/llvm/llvm-project/blob/a0b8d548fd250c92c8f9274b57e38ad3f0b215e9/clang/lib/Analysis/CFG.cpp#L5465
					if (optional<CFGBaseDtor> baseDtor = ref->getAs<CFGBaseDtor>())
					{
						const CXXBaseSpecifier *baseSpec = baseDtor->getBaseSpecifier();
						if (const RecordType *recordType = dyn_cast<RecordType>(baseSpec->getType().getDesugaredType(*m_astContext).getTypePtr()))
						{
							if (const CXXRecordDecl *recordDecl = dyn_cast<CXXRecordDecl>(recordType->getDecl()))
							{
								if (const CXXDestructorDecl *dtorDecl = recordDecl->getDestructor())
								{
									recordDestructorCall(functionDecl, dtorDecl);
								}
							}
						}
					}
					// If it were not for the above unimplemented functionality, we would only need
					// this block.
					else if (optional<CFGImplicitDtor> implicitDtor = ref->getAs<CFGImplicitDtor>())
					{
						if (const CXXDestructorDecl *dtorDecl = implicitDtor->getDestructorDecl(*m_astContext))
						{
							recordDestructorCall(functionDecl, dtorDecl);
						}
					}
				}
			}
		}
	}
}

void CxxAstVisitorComponentIndexer::visitCXXMethodDecl(clang::CXXMethodDecl* d)
{
	// Decl has been recorded in VisitFunctionDecl
	if (getAstVisitor()->shouldVisitDecl(d))
	{
		Id symbolId = getOrCreateSymbolId(d);
		ParseLocation location = getParseLocation(d->getLocation());

		for (clang::CXXMethodDecl::method_iterator it =
				 d->begin_overridden_methods();	   // TODO: iterate in traversal and use
												   // REFERENCE_OVERRIDE or so..
			 it != d->end_overridden_methods();
			 it++)
		{
			Id overrideId = getOrCreateSymbolId(*it);
			m_client->recordSymbolKind(overrideId, SymbolKind::FUNCTION);
			m_client->recordReference(REFERENCE_OVERRIDE, overrideId, symbolId, location);
		}

		// record edge from Foo::bar<int>() to Foo::bar<T>()
		recordTemplateMemberSpecialization(
			d->getMemberSpecializationInfo(), symbolId, location, SymbolKind::FUNCTION);
	}
}

void CxxAstVisitorComponentIndexer::visitEnumConstantDecl(clang::EnumConstantDecl* d)
{
	if (getAstVisitor()->shouldVisitDecl(d))
	{
		Id symbolId = getOrCreateSymbolId(d);
		m_client->recordSymbolKind(symbolId, SymbolKind::ENUM_CONSTANT);
		m_client->recordLocation(symbolId, getParseLocation(d->getLocation()), ParseLocationType::TOKEN);
		m_client->recordDefinitionKind(symbolId, utility::getDefinitionKind(d));
	}
}

void CxxAstVisitorComponentIndexer::visitNamespaceDecl(clang::NamespaceDecl* d)
{
	if (getAstVisitor()->shouldVisitDecl(d))
	{
		Id symbolId = getOrCreateSymbolId(d);
		m_client->recordSymbolKind(symbolId, SymbolKind::NAMESPACE);
		m_client->recordLocation(symbolId, getParseLocation(d->getLocation()), ParseLocationType::TOKEN);
		m_client->recordLocation(symbolId, getParseLocation(d->getSourceRange()), ParseLocationType::SCOPE);
		m_client->recordAccessKind(symbolId, utility::convertAccessSpecifier(d->getAccess()));
		m_client->recordDefinitionKind(symbolId, utility::getDefinitionKind(d));
	}
}

void CxxAstVisitorComponentIndexer::visitNamespaceAliasDecl(clang::NamespaceAliasDecl* d)
{
	if (getAstVisitor()->shouldVisitDecl(d))
	{
		Id symbolId = getOrCreateSymbolId(d);
		m_client->recordSymbolKind(symbolId, SymbolKind::NAMESPACE);
		m_client->recordLocation(symbolId, getParseLocation(d->getLocation()), ParseLocationType::TOKEN);
		m_client->recordAccessKind(symbolId, utility::convertAccessSpecifier(d->getAccess()));
		m_client->recordDefinitionKind(symbolId, utility::getDefinitionKind(d));

		m_client->recordReference(
			REFERENCE_USAGE,
			getOrCreateSymbolId(d->getAliasedNamespace()),
			symbolId,
			getParseLocation(d->getTargetNameLoc()));

		// TODO: record other namespace as undefined
	}
}

void CxxAstVisitorComponentIndexer::visitTypedefDecl(clang::TypedefDecl* d)
{
	if (getAstVisitor()->shouldVisitDecl(d))
	{
		Id symbolId = getOrCreateSymbolId(d);
		m_client->recordSymbolKind(
			symbolId,
			d->getAnonDeclWithTypedefName() == nullptr
				? SymbolKind::TYPEDEF
				: utility::convertTagKind(d->getAnonDeclWithTypedefName()->getTagKind()));
		m_client->recordLocation(symbolId, getParseLocation(d->getLocation()), ParseLocationType::TOKEN);
		m_client->recordAccessKind(symbolId, utility::convertAccessSpecifier(d->getAccess()));
		m_client->recordDefinitionKind(symbolId, utility::getDefinitionKind(d));
	}
}

void CxxAstVisitorComponentIndexer::visitTypeAliasDecl(clang::TypeAliasDecl* d)
{
	if (getAstVisitor()->shouldVisitDecl(d))
	{
		Id symbolId = getOrCreateSymbolId(d);
		m_client->recordSymbolKind(
			symbolId,
			d->getAnonDeclWithTypedefName() == nullptr
				? SymbolKind::TYPEDEF
				: utility::convertTagKind(d->getAnonDeclWithTypedefName()->getTagKind()));
		m_client->recordLocation(symbolId, getParseLocation(d->getLocation()), ParseLocationType::TOKEN);
		m_client->recordAccessKind(symbolId, utility::convertAccessSpecifier(d->getAccess()));
		m_client->recordDefinitionKind(symbolId, utility::getDefinitionKind(d));
	}
}

void CxxAstVisitorComponentIndexer::visitUsingDirectiveDecl(clang::UsingDirectiveDecl* d)
{
	if (getAstVisitor()->shouldVisitDecl(d))
	{
		Id symbolId = getOrCreateSymbolId(d->getNominatedNamespaceAsWritten());
		m_client->recordSymbolKind(symbolId, SymbolKind::NAMESPACE);

		const ParseLocation location = getParseLocation(d->getLocation());

		m_client->recordReference(
			REFERENCE_USAGE,
			symbolId,
			getOrCreateSymbolId(
				getAstVisitor()->getComponent<CxxAstVisitorComponentContext>()->getContext(),
				NameHierarchy(
					getAstVisitor()
						->getCanonicalFilePathCache()
						->getCanonicalFilePath(location.fileId)
						.str(),
					NAME_DELIMITER_FILE)),
			location);
	}
}

void CxxAstVisitorComponentIndexer::visitUsingDecl(clang::UsingDecl* d)
{
	if (getAstVisitor()->shouldVisitDecl(d))
	{
		const ParseLocation location = getParseLocation(d->getLocation());

		m_client->recordReference(
			REFERENCE_USAGE,
			getOrCreateSymbolId(d),
			getOrCreateSymbolId(
				getAstVisitor()->getComponent<CxxAstVisitorComponentContext>()->getContext(),
				NameHierarchy(
					getAstVisitor()
						->getCanonicalFilePathCache()
						->getCanonicalFilePath(location.fileId)
						.str(),
					NAME_DELIMITER_FILE)),
			location);
	}
}

void CxxAstVisitorComponentIndexer::visitNonTypeTemplateParmDecl(clang::NonTypeTemplateParmDecl* d)
{
	if (getAstVisitor()->shouldVisitDecl(d) && d->getDeclName().isIdentifier() &&
		!d->getName().empty())	  // We don't create symbols for unnamed template parameters.
	{
		m_client->recordLocalSymbol(
			getLocalSymbolName(d->getLocation()), getParseLocation(d->getLocation()));
	}
}

void CxxAstVisitorComponentIndexer::visitTemplateTypeParmDecl(clang::TemplateTypeParmDecl* d)
{
	if (getAstVisitor()->shouldVisitDecl(d) && d->getDeclName().isIdentifier() &&
		!d->getName().empty())	  // We don't create symbols for unnamed template parameters.
	{
		m_client->recordLocalSymbol(
			getLocalSymbolName(d->getLocation()), getParseLocation(d->getLocation()));
	}
}

void CxxAstVisitorComponentIndexer::visitTemplateTemplateParmDecl(clang::TemplateTemplateParmDecl* d)
{
	if (getAstVisitor()->shouldVisitDecl(d) && d->getDeclName().isIdentifier() &&
		!d->getName().empty())	  // We don't create symbols for unnamed template parameters.
	{
		m_client->recordLocalSymbol(
			getLocalSymbolName(d->getLocation()), getParseLocation(d->getLocation()));
	}
}

void CxxAstVisitorComponentIndexer::visitConceptDecl(clang::ConceptDecl *d)
{
	if (getAstVisitor()->shouldVisitDecl(d))
	{
		const Id conceptDeclId = getOrCreateSymbolId(d);

		m_client->recordSymbolKind(conceptDeclId, SymbolKind::CONCEPT);

		m_client->recordAccessKind(conceptDeclId, utility::convertAccessSpecifier(d->getAccess()));

		// Make it 'indexed':
		m_client->recordDefinitionKind(conceptDeclId, utility::getDefinitionKind(d));

		// Make it navigatable/clickable:
		m_client->recordLocation(conceptDeclId, getParseLocation(d->getLocation()), ParseLocationType::TOKEN);
	}
}

void CxxAstVisitorComponentIndexer::visitConceptSpecializationExpr(clang::ConceptSpecializationExpr *d)
{
	if (getAstVisitor()->shouldVisitStmt(d))
	{
		recordConceptReference(d);
	}
}

void CxxAstVisitorComponentIndexer::visitTypeLoc(clang::TypeLoc tl)
{
	if (tl.isNull())
	{
		return;
	}

	if ((getAstVisitor()->shouldVisitReference(tl.getBeginLoc())) &&
		(getAstVisitor()->shouldHandleTypeLoc(tl)))
	{
		if (const clang::TemplateTypeParmTypeLoc &ttptl = tl.getAs<clang::TemplateTypeParmTypeLoc>())
		{
			clang::TemplateTypeParmDecl* decl = ttptl.getDecl();
			if (decl)
			{
				m_client->recordLocalSymbol(
					getLocalSymbolName(decl->getLocation()), getParseLocation(tl.getBeginLoc()));
			}
		}
		else
		{
			if (const clang::TemplateSpecializationTypeLoc& tstl = tl.getAs<clang::TemplateSpecializationTypeLoc>())
			{
				const clang::TemplateSpecializationType* tst = tstl.getTypePtr();
				if (tst)
				{
					const clang::TemplateName tln = tst->getTemplateName();
					if (tln.isDependent()) // e.g. T<int> where the template name T depends on a template parameter
					{
						clang::TemplateDecl* decl = tln.getAsTemplateDecl();
						if (decl)
						{
							m_client->recordLocalSymbol(
								getLocalSymbolName(decl->getLocation()),
								getParseLocation(tl.getBeginLoc()));
						}
						return;
					}
				}
			}

			const Id symbolId = getOrCreateSymbolId(tl.getTypePtr());

			if (clang::dyn_cast_or_null<clang::BuiltinType>(tl.getTypePtr()))
			{
				m_client->recordSymbolKind(symbolId, SymbolKind::BUILTIN_TYPE);
				m_client->recordDefinitionKind(symbolId, DefinitionKind::EXPLICIT);
			}

			clang::SourceLocation loc;
			if (const clang::DependentNameTypeLoc &dntl = tl.getAs<clang::DependentNameTypeLoc>())
			{
				loc = dntl.getNameLoc();
			}
			else
			{
				loc = tl.getBeginLoc();
			}

			const ParseLocation parseLocation = getParseLocation(loc);

			m_client->recordReference(
				getAstVisitor()->getComponent<CxxAstVisitorComponentTypeRefKind>()->isTraversingInheritance()
					? REFERENCE_INHERITANCE
					: REFERENCE_TYPE_USAGE,
				symbolId,
				getOrCreateSymbolId(
					getAstVisitor()->getComponent<CxxAstVisitorComponentContext>()->getContext(
						1)),	// we skip the last element because it refers to this typeloc.
				parseLocation);

			if (getAstVisitor()
					->getComponent<CxxAstVisitorComponentTypeRefKind>()
					->isTraversingTemplateArgument())
			{
				if (const clang::NamedDecl* namedContextDecl =
						getAstVisitor()
							->getComponent<CxxAstVisitorComponentContext>()
							->getTopmostContextDecl(2))
				{
					m_client->recordReference(
						REFERENCE_TYPE_USAGE,
						symbolId,
						getOrCreateSymbolId(namedContextDecl),	  // we use the closest named decl here
						parseLocation);
				}
			}
		}
	}
}

void CxxAstVisitorComponentIndexer::visitDeclRefExpr(clang::DeclRefExpr* s)
{
	clang::ValueDecl* decl = s->getDecl();
	if (getAstVisitor()->shouldVisitReference(s->getLocation()))
	{
		if ((clang::isa<clang::ParmVarDecl>(decl)) ||
			(clang::isa<clang::VarDecl>(decl) && decl->getParentFunctionOrMethod() != nullptr))
		{
			if (!utility::isImplicit(decl))
			{
				m_client->recordLocalSymbol(
					getLocalSymbolName(decl->getLocation()), getParseLocation(s->getLocation()));
			}
			// else { don't do anything }
		}
		else if (
			(clang::isa<clang::NonTypeTemplateParmDecl>(decl)) ||
			(clang::isa<clang::TemplateTypeParmDecl>(decl)) ||
			(clang::isa<clang::TemplateTemplateParmDecl>(decl)))
		{
			m_client->recordLocalSymbol(
				getLocalSymbolName(decl->getLocation()), getParseLocation(s->getLocation()));
		}
		else
		{
			Id symbolId = getOrCreateSymbolId(decl);

			const ReferenceKind refKind = consumeDeclRefContextKind();
			if (refKind == REFERENCE_CALL)
			{
				m_client->recordSymbolKind(symbolId, SymbolKind::FUNCTION);
			}

			m_client->recordReference(
				refKind,
				symbolId,
				getOrCreateSymbolId(
					getAstVisitor()->getComponent<CxxAstVisitorComponentContext>()->getContext()),
				getParseLocation(s->getLocation()));
		}
	}
}

void CxxAstVisitorComponentIndexer::visitMemberExpr(clang::MemberExpr* s)
{
	if (getAstVisitor()->shouldVisitReference(s->getMemberLoc()))
	{
		Id symbolId = getOrCreateSymbolId(s->getMemberDecl());

		const ReferenceKind refKind = consumeDeclRefContextKind();
		if (refKind == REFERENCE_CALL)
		{
			m_client->recordSymbolKind(symbolId, SymbolKind::FUNCTION);
		}

		m_client->recordReference(
			refKind,
			symbolId,
			getOrCreateSymbolId(
				getAstVisitor()->getComponent<CxxAstVisitorComponentContext>()->getContext()),
			getParseLocation(s->getMemberLoc()));
	}
}

void CxxAstVisitorComponentIndexer::visitCXXConstructExpr(clang::CXXConstructExpr* s)
{
	const clang::CXXConstructorDecl* constructorDecl = s->getConstructor();

	if (!constructorDecl)
	{
		return;
	}
	else
	{
		const clang::CXXRecordDecl* parentDecl = constructorDecl->getParent();
		if (!parentDecl || parentDecl->isLambda())
		{
			return;
		}
	}

	if (getAstVisitor()->shouldVisitReference(s->getLocation()))
	{
		clang::SourceLocation loc;
		clang::SourceLocation braceBeginLoc = s->getParenOrBraceRange().getBegin();
		clang::SourceLocation nameBeginLoc = s->getSourceRange().getBegin();
		if (braceBeginLoc.isValid())
		{
			if (braceBeginLoc == nameBeginLoc)
			{
				loc = nameBeginLoc;
			}
			else
			{
				loc = braceBeginLoc.getLocWithOffset(-1);
			}
		}
		else
		{
			loc = s->getSourceRange().getEnd();
		}
		loc = clang::Lexer::GetBeginningOfToken(
			loc, m_astContext->getSourceManager(), m_astContext->getLangOpts());

		const Id symbolId = getOrCreateSymbolId(s->getConstructor());

		const ReferenceKind refKind = consumeDeclRefContextKind();
		if (refKind == REFERENCE_CALL)
		{
			m_client->recordSymbolKind(symbolId, SymbolKind::FUNCTION);
		}

		m_client->recordReference(
			refKind,
			symbolId,
			getOrCreateSymbolId(
				getAstVisitor()->getComponent<CxxAstVisitorComponentContext>()->getContext()),
			getParseLocation(loc));
	}
}

void CxxAstVisitorComponentIndexer::visitCXXDeleteExpr(clang::CXXDeleteExpr* s)
{
	if (!s->isArrayForm() && getAstVisitor()->shouldVisitReference(s->getBeginLoc()))
	{
		clang::QualType destroyedTypeQual = s->getDestroyedType();
		const clang::Type* destroyedType = destroyedTypeQual.getTypePtrOrNull();
		if (destroyedType != nullptr)
		{
			clang::CXXRecordDecl* recordDecl = destroyedType->getAsCXXRecordDecl();
			if (recordDecl != nullptr)
			{
				clang::CXXDestructorDecl* destructorDecl = recordDecl->getDestructor();
				if (destructorDecl != nullptr)
				{
					const Id symbolId = getOrCreateSymbolId(destructorDecl);

					m_client->recordReference(
						REFERENCE_CALL,
						symbolId,
						getOrCreateSymbolId(
							getAstVisitor()->getComponent<CxxAstVisitorComponentContext>()->getContext()),
						getParseLocation(s->getBeginLoc()));
				}
			}
		}
	}
}

void CxxAstVisitorComponentIndexer::visitLambdaExpr(clang::LambdaExpr* s)
{
	if (getAstVisitor()->shouldVisitStmt(s))
	{
		if (const clang::CXXMethodDecl* methodDecl = s->getCallOperator())
		{
			Id symbolId = getOrCreateSymbolId(methodDecl);
			m_client->recordSymbolKind(symbolId, SymbolKind::FUNCTION);
			m_client->recordLocation(symbolId, getParseLocation(s->getBeginLoc()), ParseLocationType::TOKEN);
			m_client->recordLocation(symbolId, getParseLocationOfFunctionBody(methodDecl), ParseLocationType::SCOPE);
			m_client->recordDefinitionKind(symbolId, utility::getDefinitionKind(methodDecl));
		}

		// Iterate over the "closure type parameters" to detect concept usages:

		if (const CXXRecordDecl *closureRecordDecl = s->getLambdaClass())
		{
			for (Decl *decl : closureRecordDecl->decls())
			{
				if (const FunctionTemplateDecl *functionTemplateDecl = dyn_cast<FunctionTemplateDecl>(decl))
				{
					if (functionTemplateDecl->getTemplatedDecl() != nullptr)
					{
						recordTemplateParameterConceptReferences(functionTemplateDecl);
					}
				}
			}
		}
	}
}

void CxxAstVisitorComponentIndexer::visitConstructorInitializer(clang::CXXCtorInitializer* init)
{
	if (getAstVisitor()->shouldVisitReference(init->getMemberLocation()))
	{
		// record the field usage here because it is not a DeclRefExpr
		if (clang::FieldDecl* memberDecl = init->getMember())
		{
			m_client->recordReference(
				REFERENCE_USAGE,
				getOrCreateSymbolId(memberDecl),
				getOrCreateSymbolId(
					getAstVisitor()->getComponent<CxxAstVisitorComponentContext>()->getContext()),
				getParseLocation(init->getMemberLocation()));
		}
	}
}

void CxxAstVisitorComponentIndexer::recordTemplateMemberSpecialization(
	const clang::MemberSpecializationInfo* memberSpecializationInfo,
	Id contextId,
	const ParseLocation& location,
	SymbolKind symbolKind)
{
	if (memberSpecializationInfo != nullptr)
	{
		Id symbolId = getOrCreateSymbolId(memberSpecializationInfo->getInstantiatedFrom());
		m_client->recordSymbolKind(symbolId, symbolKind);
		m_client->recordReference(REFERENCE_TEMPLATE_SPECIALIZATION, symbolId, contextId, location);
	}
}

ParseLocation CxxAstVisitorComponentIndexer::getSignatureLocation(clang::FunctionDecl* d)
{
	clang::SourceRange signatureRange = d->getSourceRange();

	if (d->doesThisDeclarationHaveABody())
	{
		if (!d->getTypeSourceInfo())
		{
			return ParseLocation();
		}

		const clang::SourceManager& sm = m_astContext->getSourceManager();
		const clang::LangOptions& opts = m_astContext->getLangOpts();

		clang::SourceLocation endLoc = signatureRange.getBegin();

		if (d->getNumParams() > 0)
		{
			endLoc = d->getParamDecl(d->getNumParams() - 1)->getEndLoc();
		}

		while (sm.isBeforeInTranslationUnit(endLoc, signatureRange.getEnd()))
		{
			std::optional<clang::Token> token = clang::Lexer::findNextToken(endLoc, sm, opts);
			if (token.has_value())
			{
				const clang::tok::TokenKind tokenKind = token.value().getKind();
				if (tokenKind == clang::tok::l_brace || tokenKind == clang::tok::colon)
				{
					signatureRange.setEnd(endLoc);
					return getParseLocation(signatureRange);
				}

				clang::SourceLocation nextEndLoc = token.value().getLocation();
				if (nextEndLoc == endLoc)
				{
					return ParseLocation();
				}

				endLoc = nextEndLoc;
			}
			else
			{
				return ParseLocation();
			}
		}
		return ParseLocation();
	}

	return getParseLocation(signatureRange);
}

ParseLocation CxxAstVisitorComponentIndexer::getParseLocationOfTagDeclBody(clang::TagDecl* decl) const
{
	return getAstVisitor()->getParseLocationOfTagDeclBody(decl);
}

ParseLocation CxxAstVisitorComponentIndexer::getParseLocationOfFunctionBody(
	const clang::FunctionDecl* decl) const
{
	return getAstVisitor()->getParseLocationOfFunctionBody(decl);
}

ParseLocation CxxAstVisitorComponentIndexer::getParseLocation(const clang::SourceLocation& loc) const
{
	return getAstVisitor()->getParseLocation(loc);
}

ParseLocation CxxAstVisitorComponentIndexer::getParseLocation(const clang::SourceRange& sourceRange) const
{
	return getAstVisitor()->getParseLocation(sourceRange);
}

std::string CxxAstVisitorComponentIndexer::getLocalSymbolName(const clang::SourceLocation& loc) const
{
	const ParseLocation location = getParseLocation(loc);
	return getAstVisitor()->getCanonicalFilePathCache()->getCanonicalFilePath(location.fileId).fileName() +
		"<" + std::to_string(location.startLineNumber) + ":" +
		std::to_string(location.startColumnNumber) + ">";
}

ReferenceKind CxxAstVisitorComponentIndexer::consumeDeclRefContextKind()
{
	CxxAstVisitorComponentTypeRefKind* typeRefKindComponent =
		getAstVisitor()->getComponent<CxxAstVisitorComponentTypeRefKind>();
	if (typeRefKindComponent->isTraversingInheritance())
	{
		return REFERENCE_INHERITANCE;
	}
	else if (typeRefKindComponent->isTraversingTemplateArgument())
	{
		return REFERENCE_TYPE_USAGE;
	}

	return getAstVisitor()->getComponent<CxxAstVisitorComponentDeclRefKind>()->getReferenceKind();
}

Id CxxAstVisitorComponentIndexer::getOrCreateSymbolId(const clang::NamedDecl* decl)
{
	auto it = m_declSymbolIds.find(decl);
	if (it != m_declSymbolIds.end())
	{
		return it->second;
	}

	NameHierarchy symbolName("global", NAME_DELIMITER_UNKNOWN);
	if (decl)
	{
		std::unique_ptr<CxxDeclName> declName =
			CxxDeclNameResolver(getAstVisitor()->getCanonicalFilePathCache()).getName(decl);
		if (declName)
		{
			symbolName = declName->toNameHierarchy();

			// TODO: replace duplicate main definition fix with better solution
			if (dynamic_cast<CxxFunctionDeclName*>(declName.get()) && symbolName.size() == 1 &&
				symbolName.back().getName() == "main")
			{
				NameElement::Signature sig = symbolName.back().getSignature();
				symbolName.pop();
				symbolName.push(NameElement(
					".:main:." +
						getAstVisitor()->getCanonicalFilePathCache()->getDeclarationFilePath(decl).str(),
					sig.getPrefix(),
					sig.getPostfix()));
			}
		}
	}

	Id symbolId = m_client->recordSymbol(symbolName);
	m_declSymbolIds.emplace(decl, symbolId);
	return symbolId;
}

Id CxxAstVisitorComponentIndexer::getOrCreateSymbolId(const clang::Type* type)
{
	auto it = m_typeSymbolIds.find(type);
	if (it != m_typeSymbolIds.end())
	{
		return it->second;
	}

	NameHierarchy symbolName("global", NAME_DELIMITER_UNKNOWN);
	if (type)
	{
		std::unique_ptr<CxxTypeName> typeName =
			CxxTypeNameResolver(getAstVisitor()->getCanonicalFilePathCache()).getName(type);
		if (typeName)
		{
			symbolName = typeName->toNameHierarchy();
		}
	}

	Id symbolId = m_client->recordSymbol(symbolName);
	m_typeSymbolIds.emplace(type, symbolId);
	return symbolId;
}

Id CxxAstVisitorComponentIndexer::getOrCreateSymbolId(const CxxContext* context)
{
	if (context)
	{
		if (context->getDecl())
		{
			return getOrCreateSymbolId(context->getDecl());
		}
		else
		{
			return getOrCreateSymbolId(context->getType());
		}
	}

	const clang::NamedDecl* decl {nullptr};
	return getOrCreateSymbolId(decl);
}

Id CxxAstVisitorComponentIndexer::getOrCreateSymbolId(
	const CxxContext* context, const NameHierarchy& fallback)
{
	if (context)
	{
		if (context->getDecl())
		{
			return getOrCreateSymbolId(context->getDecl());
		}
		else if (context->getType())
		{
			return getOrCreateSymbolId(context->getType());
		}
	}

	return m_client->recordSymbol(fallback);	// TODO: cache result somehow
}
