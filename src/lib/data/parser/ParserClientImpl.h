#ifndef PARSER_CLIENT_IMPL_H
#define PARSER_CLIENT_IMPL_H

#include "DefinitionKind.h"
#include "IntermediateStorage.h"
#include "LocationType.h"
#include "ParserClient.h"

#include <memory>

class ParserClientImpl: public ParserClient
{
public:
	ParserClientImpl(std::shared_ptr<IntermediateStorage> storage);

	Id recordFile(const FilePath& filePath, bool indexed) override;
	void recordFileLanguage(Id fileId, const std::string& languageIdentifier) override;

	Id recordSymbol(const NameHierarchy& symbolName) override;
	void recordSymbolKind(Id symbolId, SymbolKind symbolKind) override;
	void recordAccessKind(Id symbolId, AccessKind accessKind) override;
	void recordDefinitionKind(Id symbolId, DefinitionKind definitionKind) override;

	Id recordReference(
		ReferenceKind referenceKind,
		Id referencedSymbolId,
		Id contextSymbolId,
		const ParseLocation& location) override;

	void recordLocalSymbol(const std::string& name, const ParseLocation& location) override;
	void recordLocation(Id elementId, const ParseLocation& location, ParseLocationType type) override;
	void recordComment(const ParseLocation& location) override;

	void recordError(
		const std::string& message,
		bool fatal,
		bool indexed,
		const FilePath& translationUnit,
		const ParseLocation& location) override;

	bool hasContent() const override;

private:
	void addAccess(Id nodeId, AccessKind access);

	Id addNodeHierarchy(const NameHierarchy& nameHierarchy);
	Id addFileName(const FilePath& filePath);
	Id addEdge(Edge::EdgeType type, Id sourceId, Id targetId);

	void addSourceLocation(Id elementId, const ParseLocation& location, LocationType type);

	std::shared_ptr<IntermediateStorage> m_storage;
	std::map<std::string, Id> m_fileIdMap;
};

#endif	  // PARSER_CLIENT_IMPL_H
