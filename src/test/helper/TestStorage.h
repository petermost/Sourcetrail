#ifndef TEST_STORAGE_H
#define TEST_STORAGE_H

#include <FilePath.h>
#include <Storage.h>
#include <StorageSourceLocation.h>

#include <boost/filesystem.hpp>

#include <memory>
#include <string>
#include <vector>

class TestStorage
{
public:
	static std::shared_ptr<TestStorage> create(std::shared_ptr<const Storage> storage);

	std::vector<std::string> errors;
	std::vector<std::string> qualifiers;

	std::vector<std::string> packages;
	std::vector<std::string> typedefs;
	std::vector<std::string> builtinTypes;
	std::vector<std::string> classes;
	std::vector<std::string> unions;
	std::vector<std::string> interfaces;
	std::vector<std::string> records;
	std::vector<std::string> annotations;
	std::vector<std::string> enums;
	std::vector<std::string> enumConstants;
	std::vector<std::string> functions;
	std::vector<std::string> fields;
	std::vector<std::string> globalVariables;
	std::vector<std::string> methods;
	std::vector<std::string> namespaces;
	std::vector<std::string> modules;
	std::vector<std::string> structs;
	std::vector<std::string> macros;
	std::vector<std::string> typeParameters;
	std::vector<std::string> localSymbols;
	std::set<std::string> files;
	std::vector<std::string> comments;

	std::vector<std::string> inheritances;
	std::vector<std::string> overrides;
	std::vector<std::string> calls;
	std::vector<std::string> usages;	   // for variables
	std::vector<std::string> typeUses;	   // for types
	std::vector<std::string> macroUses;
	std::vector<std::string> annotationUses;
	std::vector<std::string> typeArguments;
	std::vector<std::string> templateSpecializations;
	std::vector<std::string> includes;
	std::vector<std::string> imports;

	std::vector<std::string> m_lines;

private:
	static std::string nodeTypeToString(NodeKind nodeType);
	static std::string edgeTypeToString(Edge::EdgeType edgeType);
	static std::string addLocationStr(const std::string& locationStr, const StorageSourceLocation& loc);
	static std::string addFileName(const std::string& locationStr, const FilePath& filePath);
	static bool containsLocation(const StorageSourceLocation& out, const StorageSourceLocation& in);
	
	std::vector<std::string>* getBinForNodeType(NodeKind nodeType);
	std::vector<std::string>* getBinForEdgeType(Edge::EdgeType edgeType);
	void addLine(const std::string& message);
};

#endif	  // TEST_STORAGE_H
