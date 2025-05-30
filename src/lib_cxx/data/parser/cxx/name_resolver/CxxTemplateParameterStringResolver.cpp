#include "CxxTemplateParameterStringResolver.h"

#include <sstream>

#include <clang/AST/DeclTemplate.h>
#include <clang/AST/PrettyPrinter.h>

#include "CxxTypeNameResolver.h"

CxxTemplateParameterStringResolver::CxxTemplateParameterStringResolver(
	CanonicalFilePathCache* canonicalFilePathCache)
	: CxxNameResolver(canonicalFilePathCache)
{
}

CxxTemplateParameterStringResolver::CxxTemplateParameterStringResolver(const CxxNameResolver* other)
	: CxxNameResolver(other)
{
}

std::string CxxTemplateParameterStringResolver::getTemplateParameterString(
	const clang::NamedDecl* parameter)
{
	std::string templateParameterTypeString;

	if (parameter)
	{
		const clang::Decl::Kind templateParameterKind = parameter->getKind();
		switch (templateParameterKind)
		{
		case clang::Decl::NonTypeTemplateParm:
			templateParameterTypeString = getTemplateParameterTypeString(
				clang::dyn_cast<clang::NonTypeTemplateParmDecl>(parameter));
			break;
		case clang::Decl::TemplateTypeParm:
			templateParameterTypeString = getTemplateParameterTypeString(
				clang::dyn_cast<clang::TemplateTypeParmDecl>(parameter));
			break;
		case clang::Decl::TemplateTemplateParm:
			templateParameterTypeString = getTemplateParameterTypeString(
				clang::dyn_cast<clang::TemplateTemplateParmDecl>(parameter));
			break;
		default:
			// LOG_ERROR("Unhandled kind of template parameter.");
			break;
		}

		const std::string parameterName = parameter->getName().str();
		if (!parameterName.empty())
		{
			templateParameterTypeString += ' ' + parameterName;
		}
	}
	return templateParameterTypeString;
}

std::string CxxTemplateParameterStringResolver::getTemplateParameterTypeString(
	const clang::NonTypeTemplateParmDecl* parameter)
{
	std::string typeString = CxxTypeName::makeUnsolvedIfNull(
								  CxxTypeNameResolver(this).getName(parameter->getType()))
								  ->toString();

	if (parameter->isTemplateParameterPack())
	{
		typeString += "...";
	}

	return typeString;
}

std::string CxxTemplateParameterStringResolver::getTemplateParameterTypeString(
	const clang::TemplateTypeParmDecl* parameter)
{
	std::string typeString = (parameter->wasDeclaredWithTypename() ? "typename" : "class");

	if (parameter->isTemplateParameterPack())
	{
		typeString += "...";
	}

	return typeString;
}

std::string CxxTemplateParameterStringResolver::getTemplateParameterTypeString(
	const clang::TemplateTemplateParmDecl* parameter)
{
	std::stringstream ss;
	ss << "template<";
	const clang::TemplateParameterList* parameterList = parameter->getTemplateParameters();
	for (unsigned i = 0; i < parameterList->size(); i++)
	{
		if (i > 0)
		{
			ss << ", ";
		}

		CxxTemplateParameterStringResolver parameterStringResolver(this);
		parameterStringResolver.ignoreContextDecl(parameterList->getParam(i));

		ss << parameterStringResolver.getTemplateParameterString(parameterList->getParam(i));
	}
	ss << "> typename";	// TODO: what if template template parameter is defined with class keyword?

	if (parameter->isTemplateParameterPack())
	{
		ss << "...";
	}

	return ss.str();
}
