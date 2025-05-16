#include "Catch2.hpp"

#include "language_packages.h"

#if BUILD_CXX_LANGUAGE_PACKAGE

#	include "CxxTypeName.h"

TEST_CASE("type name created with name has no qualifiers or modifiers")
{
	CxxTypeName typeName("int", std::vector<std::string>(), std::shared_ptr<CxxName>());
	REQUIRE("int" == typeName.toString());
}

TEST_CASE("type name created with name and const qualifier has no modifiers")
{
	CxxTypeName typeName("int", std::vector<std::string>(), std::shared_ptr<CxxName>());
	typeName.addQualifier(CxxQualifierFlags::QUALIFIER_CONST);
	REQUIRE("const int" == typeName.toString());
}

TEST_CASE("type name created with name and array modifier has array modifier")
{
	CxxTypeName typeName("int", std::vector<std::string>(), std::shared_ptr<CxxName>());
	typeName.addModifier(CxxTypeName::Modifier("[]"));
	REQUIRE("int []" == typeName.toString());
}

TEST_CASE("type name created with name and const pointer modifier has const pointer modifier")
{
	CxxTypeName typeName("int", std::vector<std::string>(), std::shared_ptr<CxxName>());
	typeName.addModifier(CxxTypeName::Modifier("*"));
	typeName.addQualifier(CxxQualifierFlags::QUALIFIER_CONST);
	REQUIRE("int * const" == typeName.toString());
}

TEST_CASE("type name created with name and pointer pointer modifier has pointer pointer modifier")
{
	CxxTypeName typeName("int", std::vector<std::string>(), std::shared_ptr<CxxName>());
	typeName.addModifier(CxxTypeName::Modifier("*"));
	typeName.addModifier(CxxTypeName::Modifier("*"));
	REQUIRE("int * *" == typeName.toString());
}

#endif	  // BUILD_CXX_LANGUAGE_PACKAGE
