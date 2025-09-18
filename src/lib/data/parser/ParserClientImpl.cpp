#include "ParserClientImpl.h"

#include "Edge.h"
#include "NodeKind.h"
#include "ParseLocation.h"

namespace
{

NodeKind symbolKindToNodeKind(SymbolKind symbolKind) 
{
	switch (symbolKind)
	{
		case SymbolKind::ANNOTATION:
			return NODE_ANNOTATION;
		case SymbolKind::BUILTIN_TYPE:
			return NODE_BUILTIN_TYPE;
		case SymbolKind::CLASS:
			return NODE_CLASS;
		case SymbolKind::ENUM:
			return NODE_ENUM;
		case SymbolKind::ENUM_CONSTANT:
			return NODE_ENUM_CONSTANT;
		case SymbolKind::FIELD:
			return NODE_FIELD;
		case SymbolKind::FUNCTION:
			return NODE_FUNCTION;
		case SymbolKind::GLOBAL_VARIABLE:
			return NODE_GLOBAL_VARIABLE;
		case SymbolKind::INTERFACE:
			return NODE_INTERFACE;
		case SymbolKind::MACRO:
			return NODE_MACRO;
		case SymbolKind::METHOD:
			return NODE_METHOD;
		case SymbolKind::MODULE:
			return NODE_MODULE;
		case SymbolKind::NAMESPACE:
			return NODE_NAMESPACE;
		case SymbolKind::PACKAGE:
			return NODE_PACKAGE;
		case SymbolKind::STRUCT:
			return NODE_STRUCT;
		case SymbolKind::TYPEDEF:
			return NODE_TYPEDEF;
		case SymbolKind::TYPE_PARAMETER:
			return NODE_TYPE_PARAMETER;
		case SymbolKind::UNION:
			return NODE_UNION;
		case SymbolKind::RECORD:
			return NODE_RECORD;
		case SymbolKind::CONCEPT:
			return NODE_CONCEPT;
		default:
			break;
	}
	return NODE_SYMBOL;
}

Edge::EdgeType referenceKindToEdgeType(ReferenceKind referenceKind) 
{
	switch (referenceKind)
	{
		case REFERENCE_TYPE_USAGE:
			return Edge::EDGE_TYPE_USAGE;
		case REFERENCE_USAGE:
			return Edge::EDGE_USAGE;
		case REFERENCE_CALL:
			return Edge::EDGE_CALL;
		case REFERENCE_INHERITANCE:
			return Edge::EDGE_INHERITANCE;
		case REFERENCE_OVERRIDE:
			return Edge::EDGE_OVERRIDE;
		case REFERENCE_TYPE_ARGUMENT:
			return Edge::EDGE_TYPE_ARGUMENT;
		case REFERENCE_TEMPLATE_SPECIALIZATION:
			return Edge::EDGE_TEMPLATE_SPECIALIZATION;
		case REFERENCE_INCLUDE:
			return Edge::EDGE_INCLUDE;
		case REFERENCE_IMPORT:
			return Edge::EDGE_IMPORT;
		case REFERENCE_MACRO_USAGE:
			return Edge::EDGE_MACRO_USAGE;
		case REFERENCE_ANNOTATION_USAGE:
			return Edge::EDGE_ANNOTATION_USAGE;
		default:
			break;
	}
	return Edge::EDGE_UNDEFINED;
}

LocationType parseLocationTypeToLocationType(ParseLocationType type) 
{
	switch (type)
	{
		case ParseLocationType::TOKEN:
			return LocationType::TOKEN;
		case ParseLocationType::SCOPE:
			return LocationType::SCOPE;
		case ParseLocationType::SIGNATURE:
			return LocationType::SIGNATURE;
		case ParseLocationType::QUALIFIER:
			return LocationType::QUALIFIER;
		case ParseLocationType::LOCAL:
			return LocationType::LOCAL_SYMBOL;
	}
	return LocationType::TOKEN;
}

}

ParserClientImpl::ParserClientImpl(std::shared_ptr<IntermediateStorage> storage): m_storage(storage) {}

Id ParserClientImpl::recordFile(const FilePath& filePath, bool indexed)
{
	Id fileId = addFileName(filePath);
	m_storage->addFile(StorageFile(fileId, filePath.str(), "", "", indexed, true));
	return fileId;
}

void ParserClientImpl::recordFileLanguage(Id fileId, const std::string& languageIdentifier)
{
	m_storage->setFileLanguage(fileId, languageIdentifier);
}

Id ParserClientImpl::recordSymbol(const NameHierarchy& symbolName)
{
	return addNodeHierarchy(symbolName);
}

void ParserClientImpl::recordSymbolKind(Id symbolId, SymbolKind symbolKind)
{
	m_storage->setNodeType(symbolId, symbolKindToNodeKind(symbolKind));
}

void ParserClientImpl::recordAccessKind(Id symbolId, AccessKind accessKind)
{
	if (accessKind != AccessKind::NONE)
	{
		m_storage->addComponentAccess(StorageComponentAccess(symbolId, accessKind));
	}
}

void ParserClientImpl::recordDefinitionKind(Id symbolId, DefinitionKind definitionKind)
{
	if (definitionKind != DefinitionKind::NONE)
	{
		m_storage->addSymbol(StorageSymbol(symbolId, definitionKind));
	}
}

Id ParserClientImpl::recordReference(
	ReferenceKind referenceKind, Id referencedSymbolId, Id contextSymbolId, const ParseLocation& location)
{
	Id edgeId = addEdge(referenceKindToEdgeType(referenceKind), contextSymbolId, referencedSymbolId);
	if (edgeId)
	{
		addSourceLocation(edgeId, location, LocationType::TOKEN);
	}
	return edgeId;
}

void ParserClientImpl::recordLocalSymbol(const std::string& name, const ParseLocation& location)
{
	const Id localSymbolId = m_storage->addLocalSymbol(name);
	addSourceLocation(localSymbolId, location, LocationType::LOCAL_SYMBOL);
}

void ParserClientImpl::recordLocation(Id elementId, const ParseLocation& location, ParseLocationType type)
{
	addSourceLocation(elementId, location, parseLocationTypeToLocationType(type));
}

void ParserClientImpl::recordComment(const ParseLocation& location)
{
	if (!location.isValid())
	{
		return;
	}

	m_storage->addSourceLocation(StorageSourceLocationData(
		location.fileId,
		location.startLineNumber,
		location.startColumnNumber,
		location.endLineNumber,
		location.endColumnNumber,
		LocationType::COMMENT));
}

void ParserClientImpl::recordError(
	const std::string& message,
	bool fatal,
	bool indexed,
	const FilePath& translationUnit,
	const ParseLocation& location)
{
	if (location.fileId != 0)
	{
		Id errorId = m_storage->addError(
			StorageErrorData(message, translationUnit.str(), fatal, indexed));

		addSourceLocation(errorId, location, LocationType::ERROR);
	}
}

bool ParserClientImpl::hasContent() const
{
	return m_storage->getByteSize(1) > 0;
}


Id ParserClientImpl::addNodeHierarchy(const NameHierarchy& nameHierarchy)
{
	Id childNodeId = 0;
	Id firstNodeId = 0;
	for (size_t i = nameHierarchy.size(); i > 0; i--)
	{
		std::pair<Id, bool> ret = m_storage->addNode(StorageNodeData(NODE_SYMBOL, 
			NameHierarchy::serializeRange(nameHierarchy, 0, i)));

		if (!firstNodeId)
		{
			firstNodeId = ret.first;
		}

		if (childNodeId != 0)
		{
			addEdge(Edge::EDGE_MEMBER, ret.first, childNodeId);
		}

		if (!ret.second)
		{
			return firstNodeId;
		}

		childNodeId = ret.first;
	}
	return firstNodeId;
}

Id ParserClientImpl::addFileName(const FilePath& filePath)
{
	const std::string file = filePath.str();

	auto it = m_fileIdMap.find(file);
	if (it != m_fileIdMap.end())
	{
		return it->second;
	}

	const Id fileId = addNodeHierarchy(NameHierarchy(file, NAME_DELIMITER_FILE));
	m_storage->setNodeType(fileId, NODE_FILE);

	m_fileIdMap.emplace(file, fileId);
	return fileId;
}

Id ParserClientImpl::addEdge(Edge::EdgeType type, Id sourceId, Id targetId)
{
	if (sourceId && targetId)
	{
		return m_storage->addEdge(StorageEdgeData(type, sourceId, targetId));
	}
	return 0;
}

void ParserClientImpl::addSourceLocation(Id elementId, const ParseLocation& location, LocationType type)
{
	if (!location.isValid())
	{
		return;
	}

	Id sourceLocationId = m_storage->addSourceLocation(StorageSourceLocationData(
		location.fileId,
		location.startLineNumber,
		location.startColumnNumber,
		location.endLineNumber,
		location.endColumnNumber,
		type));

	m_storage->addOccurrence(StorageOccurrence(elementId, sourceLocationId));
}
