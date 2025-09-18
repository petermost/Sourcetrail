#include "TestStorage.h"

#include "AccessKind.h"
#include "Edge.h"
#include "LocationType.h"
#include "NameHierarchy.h"
#include "NodeKind.h"

#include <map>

std::shared_ptr<TestStorage> TestStorage::create(std::shared_ptr<const Storage> storage)
{
	std::shared_ptr<TestStorage> testStorage = std::make_shared<TestStorage>();

	std::map<Id, FilePath> filePathMap;
	for (const StorageFile& file: storage->getStorageFiles())
	{
		filePathMap.emplace(file.id, FilePath(file.filePath));
		testStorage->files.emplace(file.filePath);

		testStorage->addLine(
			"FILE: " + FilePath(file.filePath).fileName() +
			(file.indexed ? "" : " non-indexed"));
	}

	std::map<Id, StorageComponentAccess> accessMap;
	for (const StorageComponentAccess& access: storage->getComponentAccesses())
	{
		accessMap.emplace(access.nodeId, access);
	}

	std::multimap<Id, Id> occurrenceMap;
	for (const StorageOccurrence& occurrence: storage->getStorageOccurrences())
	{
		occurrenceMap.emplace(occurrence.sourceLocationId, occurrence.elementId);
	}

	std::multimap<Id, StorageSourceLocation> tokenLocationMap;
	std::multimap<Id, StorageSourceLocation> scopeLocationMap;
	std::multimap<Id, StorageSourceLocation> signatureLocationMap;
	std::multimap<Id, StorageSourceLocation> localSymbolLocationMap;
	std::multimap<Id, StorageSourceLocation> qualifierLocationMap;
	std::multimap<Id, StorageSourceLocation> errorLocationMap;
	std::vector<StorageSourceLocation> commentLocations;
	for (const StorageSourceLocation& location: storage->getStorageSourceLocations())
	{
		std::vector<Id> elementIds;
		for (auto it = occurrenceMap.find(location.id);
			 it != occurrenceMap.end() && it->first == location.id;
			 it++)
		{
			elementIds.emplace_back(it->second);
		}

		if (elementIds.empty())
		{
			elementIds.emplace_back(0);
		}

		for (Id elementId: elementIds)
		{
			switch (location.type)
			{
			case LocationType::TOKEN:
				if (elementId)
				{
					tokenLocationMap.emplace(elementId, location);
				}
				break;
			case LocationType::SCOPE:
				if (elementId)
				{
					scopeLocationMap.emplace(elementId, location);
				}
				break;
			case LocationType::QUALIFIER:
				if (elementId)
				{
					qualifierLocationMap.emplace(elementId, location);
				}
				break;
			case LocationType::LOCAL_SYMBOL:
				if (elementId)
				{
					localSymbolLocationMap.emplace(elementId, location);
				}
				break;
			case LocationType::SIGNATURE:
				if (elementId)
				{
					signatureLocationMap.emplace(elementId, location);
				}
				break;
			case LocationType::ERROR:
				if (elementId)
				{
					errorLocationMap.emplace(elementId, location);
				}
				break;
			case LocationType::COMMENT:
				commentLocations.emplace_back(location);
				break;
			default:
				break;
			}
		}
	}

	std::map<Id, StorageNode> nodesMap;
	std::set<Id> fileIdMap;
	for (const StorageNode& node: storage->getStorageNodes())
	{
		nodesMap.emplace(node.id, node);

		if (node.type == NODE_FILE)
		{
			fileIdMap.insert(node.id);
		}

		std::string nameStr =
			NameHierarchy::deserialize(node.serializedName).getQualifiedNameWithSignature();

		for (auto qualifierLocationIt = qualifierLocationMap.find(node.id);
			 qualifierLocationIt != qualifierLocationMap.end() &&
			 qualifierLocationIt->first == node.id;
			 qualifierLocationIt++)
		{
			std::string locStr = addLocationStr("", qualifierLocationIt->second);
			testStorage->qualifiers.emplace_back(nameStr + locStr);
			testStorage->addLine(
				"QUALIFIER: " + nameStr +
				addFileName(locStr, filePathMap[qualifierLocationIt->second.fileNodeId]));
		}

		std::vector<std::string>* bin = testStorage->getBinForNodeType(node.type);
		if (bin != nullptr)
		{
			auto accessIt = accessMap.find(node.id);
			if (accessIt != accessMap.end())
			{
				if (accessIt->second.type != AccessKind::TEMPLATE_PARAMETER &&
					accessIt->second.type != AccessKind::TYPE_PARAMETER)
				{
					nameStr = accessKindToString(accessIt->second.type) + ' ' + nameStr;
				}
			}

			bool added = false;
			for (auto tokenLocationIt = tokenLocationMap.find(node.id);
				 tokenLocationIt != tokenLocationMap.end() && tokenLocationIt->first == node.id;
				 tokenLocationIt++)
			{
				added = false;
				std::string locationStr = addLocationStr("", tokenLocationIt->second);

				auto signatureLocationIt = signatureLocationMap.find(node.id);
				if (signatureLocationIt != signatureLocationMap.end() &&
					containsLocation(signatureLocationIt->second, tokenLocationIt->second))
				{
					locationStr = addLocationStr(locationStr, signatureLocationIt->second);
				}

				for (auto scopeLocationIt = scopeLocationMap.find(node.id);
					 scopeLocationIt != scopeLocationMap.end() &&
					 scopeLocationIt->first == node.id;
					 scopeLocationIt++)
				{
					if (containsLocation(scopeLocationIt->second, tokenLocationIt->second))
					{
						std::string locStr = addLocationStr(
							locationStr, scopeLocationIt->second);
						bin->emplace_back(nameStr + locStr);
						testStorage->addLine(
							nodeTypeToString(node.type) + ": " + nameStr +
							addFileName(locStr, filePathMap[tokenLocationIt->second.fileNodeId]));
						added = true;
					}
				}

				if (!added)
				{
					bin->emplace_back(nameStr + locationStr);
					testStorage->addLine(
						nodeTypeToString(node.type) + ": " + nameStr +
						addFileName(locationStr, filePathMap[tokenLocationIt->second.fileNodeId]));
					added = true;
				}
			}

			if (!added)
			{
				bin->emplace_back(nameStr);
				testStorage->addLine(nodeTypeToString(node.type) + ": " + nameStr);
			}
		}
	}

	for (const StorageEdge& edge: storage->getStorageEdges())
	{
		auto targetIt = nodesMap.find(edge.targetNodeId);
		if (targetIt == nodesMap.end())
		{
			continue;
		}

		auto sourceIt = nodesMap.find(edge.sourceNodeId);
		if (sourceIt == nodesMap.end())
		{
			continue;
		}

		const StorageNode& target = targetIt->second;
		const StorageNode& source = sourceIt->second;

		std::vector<std::string>* bin = testStorage->getBinForEdgeType(edge.type);
		if (bin != nullptr)
		{
			std::string sourceName =
				NameHierarchy::deserialize(source.serializedName).getQualifiedNameWithSignature();
			if (fileIdMap.find(edge.sourceNodeId) != fileIdMap.end() &&
				FilePath(sourceName).exists())
			{
				sourceName = FilePath(sourceName).fileName();
			}

			std::string targetName =
				NameHierarchy::deserialize(target.serializedName).getQualifiedNameWithSignature();
			if (fileIdMap.find(edge.targetNodeId) != fileIdMap.end() &&
				FilePath(targetName).exists())
			{
				targetName = FilePath(targetName).fileName();
			}

			std::string nameStr = sourceName + " -> " + targetName;

			bool added = false;
			for (auto tokenLocationIt = tokenLocationMap.find(edge.id);
				 tokenLocationIt != tokenLocationMap.end() && tokenLocationIt->first == edge.id;
				 tokenLocationIt++)
			{
				std::string locStr = addLocationStr("", tokenLocationIt->second);
				bin->emplace_back(nameStr + locStr);
				testStorage->addLine(
					edgeTypeToString(edge.type) + ": " + nameStr +
					addFileName(locStr, filePathMap[tokenLocationIt->second.fileNodeId]));
				added = true;
			}

			if (!added)
			{
				bin->emplace_back(nameStr);
				testStorage->addLine(edgeTypeToString(edge.type) + ": " + nameStr);
			}
		}
	}

	for (const StorageLocalSymbol& localSymbol: storage->getStorageLocalSymbols())
	{
		bool added = false;
		for (auto localSymbolLocationIt = localSymbolLocationMap.find(localSymbol.id);
			 localSymbolLocationIt != localSymbolLocationMap.end() &&
			 localSymbolLocationIt->first == localSymbol.id;
			 localSymbolLocationIt++)
		{
			std::string locStr = addLocationStr("", localSymbolLocationIt->second);
			testStorage->localSymbols.emplace_back(localSymbol.name + locStr);
			testStorage->addLine(
				"LOCAL_SYMBOL: " + localSymbol.name +
				addFileName(locStr, filePathMap[localSymbolLocationIt->second.fileNodeId]));
			added = true;
		}

		if (!added)
		{
			testStorage->localSymbols.emplace_back(localSymbol.name);
			testStorage->addLine("LOCAL_SYMBOL: " + localSymbol.name);
		}
	}

	for (const StorageSourceLocation& location: commentLocations)
	{
		std::string locStr = addLocationStr("", location);
		testStorage->comments.emplace_back("comment" + locStr);
		testStorage->addLine(
			"COMMENT: comment" + addFileName(locStr, filePathMap[location.fileNodeId]));
	}

	for (const StorageError& error: storage->getErrors())
	{
		for (auto errorLocationIt = errorLocationMap.find(error.id);
			 errorLocationIt != errorLocationMap.end() && errorLocationIt->first == error.id;
			 errorLocationIt++)
		{
			std::string locStr = addLocationStr("", errorLocationIt->second);
			testStorage->errors.emplace_back(error.message + locStr);
			testStorage->addLine(
				"ERROR: " + error.message +
				addFileName(locStr, filePathMap[errorLocationIt->second.fileNodeId]));
		}
	}
	return testStorage;
}

std::string TestStorage::nodeTypeToString(NodeKind nodeType)
{
	switch (nodeType)
	{
	case NODE_BUILTIN_TYPE:
		return "SYMBOL_BUILTIN_TYPE";
	case NODE_CLASS:
		return "SYMBOL_CLASS";
	case NODE_ENUM:
		return "SYMBOL_ENUM";
	case NODE_ENUM_CONSTANT:
		return "SYMBOL_ENUM_CONSTANT";
	case NODE_FIELD:
		return "SYMBOL_FIELD";
	case NODE_FUNCTION:
		return "SYMBOL_FUNCTION";
	case NODE_GLOBAL_VARIABLE:
		return "SYMBOL_GLOBAL_VARIABLE";
	case NODE_INTERFACE:
		return "SYMBOL_INTERFACE";
	case NODE_MACRO:
		return "SYMBOL_MACRO";
	case NODE_METHOD:
		return "SYMBOL_METHOD";
	case NODE_MODULE:
		return "SYMBOL_MODULE";
	case NODE_NAMESPACE:
		return "SYMBOL_NAMESPACE";
	case NODE_PACKAGE:
		return "SYMBOL_PACKAGE";
	case NODE_STRUCT:
		return "SYMBOL_STRUCT";
	case NODE_TYPEDEF:
		return "SYMBOL_TYPEDEF";
	case NODE_TYPE_PARAMETER:
		return "SYMBOL_TYPE_PARAMETER";
	case NODE_UNION:
		return "SYMBOL_UNION";
	case NODE_RECORD:
		return "SYMBOL_RECORD";
	case NODE_CONCEPT:
		return "SYMBOL_CONCEPT";
	default:
		break;
	}
	return "SYMBOL_NON_INDEXED";
}

std::string TestStorage::edgeTypeToString(Edge::EdgeType edgeType)
{
	switch (edgeType)
	{
	case Edge::EDGE_TYPE_USAGE:
		return "REFERENCE_TYPE_USAGE";
	case Edge::EDGE_USAGE:
		return "REFERENCE_USAGE";
	case Edge::EDGE_CALL:
		return "REFERENCE_CALL";
	case Edge::EDGE_INHERITANCE:
		return "REFERENCE_INHERITANCE";
	case Edge::EDGE_OVERRIDE:
		return "REFERENCE_OVERRIDE";
	case Edge::EDGE_TYPE_ARGUMENT:
		return "REFERENCE_TYPE_ARGUMENT";
	case Edge::EDGE_TEMPLATE_SPECIALIZATION:
		return "REFERENCE_TEMPLATE_SPECIALIZATION";
	case Edge::EDGE_INCLUDE:
		return "REFERENCE_INCLUDE";
	case Edge::EDGE_IMPORT:
		return "REFERENCE_IMPORT";
	case Edge::EDGE_MACRO_USAGE:
		return "REFERENCE_MACRO_USAGE";
	case Edge::EDGE_ANNOTATION_USAGE:
		return "REFERENCE_ANNOTATION_USAGE";
	default:
		break;
	}
	return "REFERENCE_UNDEFINED";
}

std::vector<std::string> *TestStorage::getBinForNodeType(NodeKind nodeType)
{
	switch (nodeType)
	{
	case NODE_PACKAGE:
		return &packages;
	case NODE_TYPEDEF:
		return &typedefs;
	case NODE_BUILTIN_TYPE:
		return &builtinTypes;
	case NODE_CLASS:
		return &classes;
	case NODE_UNION:
		return &unions;
	case NODE_INTERFACE:
		return &interfaces;
	case NODE_ANNOTATION:
		return &annotations;
	case NODE_ENUM:
		return &enums;
	case NODE_ENUM_CONSTANT:
		return &enumConstants;
	case NODE_FUNCTION:
		return &functions;
	case NODE_FIELD:
		return &fields;
	case NODE_GLOBAL_VARIABLE:
		return &globalVariables;
	case NODE_METHOD:
		return &methods;
	case NODE_MODULE:
		return &modules;
	case NODE_NAMESPACE:
		return &namespaces;
	case NODE_STRUCT:
		return &structs;
	case NODE_MACRO:
		return &macros;
	case NODE_TYPE_PARAMETER:
		return &typeParameters;
	case NODE_RECORD:
		return &records;
	case NODE_CONCEPT:
		return &concepts;
	default:
		break;
	}
	return nullptr;
}

std::vector<std::string> *TestStorage::getBinForEdgeType(Edge::EdgeType edgeType)
{
	switch (edgeType)
	{
	case Edge::EDGE_TYPE_USAGE:
		return &typeUses;
	case Edge::EDGE_USAGE:
		return &usages;
	case Edge::EDGE_CALL:
		return &calls;
	case Edge::EDGE_INHERITANCE:
		return &inheritances;
	case Edge::EDGE_OVERRIDE:
		return &overrides;
	case Edge::EDGE_TYPE_ARGUMENT:
		return &typeArguments;
	case Edge::EDGE_TEMPLATE_SPECIALIZATION:
		return &templateSpecializations;
	case Edge::EDGE_INCLUDE:
		return &includes;
	case Edge::EDGE_IMPORT:
		return &imports;
	case Edge::EDGE_MACRO_USAGE:
		return &macroUses;
	case Edge::EDGE_ANNOTATION_USAGE:
		return &annotationUses;
	default:
		break;
	}
	return nullptr;
}

std::string TestStorage::addLocationStr(const std::string &locationStr, const StorageSourceLocation &loc)
{
	return " <" + std::to_string(loc.startLine) + ':' + std::to_string(loc.startCol) +
		   locationStr + ' ' + std::to_string(loc.endLine) + ':' + std::to_string(loc.endCol) +
		   '>';
}

std::string TestStorage::addFileName(const std::string &locationStr, const FilePath &filePath)
{
	return " [" + filePath.fileName() + locationStr + ']';
}

bool TestStorage::containsLocation(const StorageSourceLocation &out, const StorageSourceLocation &in)
{
	if (out.startLine > in.startLine)
	{
		return false;
	}

	if (out.startLine == in.startLine && out.startCol > in.startCol)
	{
		return false;
	}

	if (out.endLine < in.endLine)
	{
		return false;
	}

	if (out.endLine == in.endLine && out.endCol < in.endCol)
	{
		return false;
	}

	return true;
}

void TestStorage::addLine(const std::string &message)
{
	m_lines.emplace_back(message + '\n');
}
