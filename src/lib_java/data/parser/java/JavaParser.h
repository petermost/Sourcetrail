#ifndef JAVA_PARSER_H
#define JAVA_PARSER_H

#include "FilePath.h"
#include "Id.h"
#include "IndexerCommandJava.h"
#include "IndexerStateInfo.h"
#include "JavaEnvironment.h"
#include "Parser.h"

#include <jni.h>

#include <functional>
#include <map>
#include <shared_mutex>
#include <string>

class FilePath;
class TextAccess;

class JavaParser: public Parser
{
public:
	static void clearCaches();

	JavaParser(std::shared_ptr<ParserClient> client, std::shared_ptr<IndexerStateInfo> indexerStateInfo);
	~JavaParser() override;

	void buildIndex(std::shared_ptr<IndexerCommandJava> indexerCommand);
	void buildIndex(const FilePath& filePath, std::shared_ptr<TextAccess> textAccess);

private:
	void buildIndex(
		const FilePath& sourceFilePath,
		const std::string& languageStandard,
		const std::string& classPath,
		std::shared_ptr<TextAccess> textAccess);

	static void dispatchToParser(jint parserId, const std::function<void (JavaParser &)> &);

	static void LogInfo(JNIEnv * /*env*/, jobject /*objectOrClass*/, jint parserId, jstring jInfo);
	static void LogWarning(JNIEnv * /*env*/, jobject /*objectOrClass*/, jint parserId, jstring jWarning);
	static void LogError(JNIEnv * /*env*/, jobject /*objectOrClass*/, jint parserId, jstring jError);

	static void RecordSymbol(JNIEnv * /*env*/, jobject /*objectOrClass*/, jint parserId, jstring jSymbolName, jint jSymbolKind, jint jAccess,
		jint jDefinitionKind);

	static void RecordSymbolWithLocation(JNIEnv * /*env*/, jobject /*objectOrClass*/, jint parserId, jstring jSymbolName, jint jSymbolKind,
		jint beginLine, jint beginColumn, jint endLine, jint endColumn, jint jAccess, jint jDefinitionKind);

	static void RecordSymbolWithLocationAndScope(JNIEnv * /*env*/, jobject /*objectOrClass*/, jint parserId, jstring jSymbolName, jint jSymbolKind,
		jint beginLine, jint beginColumn, jint endLine, jint endColumn, jint scopeBeginLine, jint scopeBeginColumn, jint scopeEndLine,
		jint scopeEndColumn, jint jAccess, jint jDefinitionKind);

	static void RecordSymbolWithLocationAndScopeAndSignature(JNIEnv * /*env*/, jobject /*objectOrClass*/, jint parserId, jstring jSymbolName,
		jint jSymbolKind, jint beginLine, jint beginColumn, jint endLine, jint endColumn, jint scopeBeginLine, jint scopeBeginColumn,
		jint scopeEndLine, jint scopeEndColumn, jint signatureBeginLine, jint signatureBeginColumn, jint signatureEndLine, jint signatureEndColumn,
		jint jAccess, jint jDefinitionKind);

	static void RecordReference(JNIEnv * /*env*/, jobject /*objectOrClass*/, jint parserId, jint jReferenceKind, jstring jReferencedName,
		jstring jContextName, jint beginLine, jint beginColumn, jint endLine, jint endColumn);

	static void RecordQualifierLocation(JNIEnv * /*env*/, jobject /*objectOrClass*/, jint parserId, jstring jQualifierName, jint beginLine,
		jint beginColumn, jint endLine, jint endColumn);

	static void RecordLocalSymbol(JNIEnv * /*env*/, jobject /*objectOrClass*/, jint parserId, jstring jSymbolName, jint beginLine, jint beginColumn,
		jint endLine, jint endColumn);

	static void RecordComment(JNIEnv * /*env*/, jobject /*objectOrClass*/, jint parserId, jint beginLine, jint beginColumn, jint endLine,
		jint endColumn);

	static void RecordError(JNIEnv * /*env*/, jobject /*objectOrClass*/, jint parserId, jstring jMessage, jint jFatal, jint jIndexed, jint beginLine,
		jint beginColumn, jint endLine, jint endColumn);

	static bool GetInterrupted(JNIEnv*  /*env*/, jobject  /*objectOrClass*/, jint parserId);

	static int s_nextParserId;
	static std::map<int, JavaParser*> s_parsers;
	static std::shared_mutex s_parsersMutex;


	bool doGetInterrupted();

	void doLogInfo(jstring jInfo);

	void doLogWarning(jstring jWarning);

	void doLogError(jstring jError);

	void doRecordSymbol(jstring jSymbolName, jint jSymbolKind, jint jAccess, jint jDefinitionKind);

	void doRecordSymbolWithLocation(
		jstring jSymbolName,
		jint jSymbolKind,
		jint beginLine,
		jint beginColumn,
		jint endLine,
		jint endColumn,
		jint jAccess,
		jint jDefinitionKind);

	void doRecordSymbolWithLocationAndScope(
		jstring jSymbolName,
		jint jSymbolKind,
		jint beginLine,
		jint beginColumn,
		jint endLine,
		jint endColumn,
		jint scopeBeginLine,
		jint scopeBeginColumn,
		jint scopeEndLine,
		jint scopeEndColumn,
		jint jAccess,
		jint jDefinitionKind);

	void doRecordSymbolWithLocationAndScopeAndSignature(
		jstring jSymbolName,
		jint jSymbolKind,
		jint beginLine,
		jint beginColumn,
		jint endLine,
		jint endColumn,
		jint scopeBeginLine,
		jint scopeBeginColumn,
		jint scopeEndLine,
		jint scopeEndColumn,
		jint signatureBeginLine,
		jint signatureBeginColumn,
		jint signatureEndLine,
		jint signatureEndColumn,
		jint jAccess,
		jint jDefinitionKind);

	void doRecordReference(
		jint jReferenceKind,
		jstring jReferencedName,
		jstring jContextName,
		jint beginLine,
		jint beginColumn,
		jint endLine,
		jint endColumn);
	void doRecordQualifierLocation(
		jstring jQualifierName, jint beginLine, jint beginColumn, jint endLine, jint endColumn);
	void doRecordLocalSymbol(
		jstring jSymbolName, jint beginLine, jint beginColumn, jint endLine, jint endColumn);
	void doRecordComment(jint beginLine, jint beginColumn, jint endLine, jint endColumn);
	void doRecordError(
		jstring jMessage,
		jint jFatal,
		jint jIndexed,
		jint beginLine,
		jint beginColumn,
		jint endLine,
		jint endColumn);

	Id getOrCreateSymbolId(jstring jSymbolName);

	std::shared_ptr<JavaEnvironment> m_javaEnvironment;
	std::shared_ptr<IndexerStateInfo> m_indexerStateInfo;
	const int m_id;

	FilePath m_currentFilePath;
	Id m_currentFileId;

	std::map<std::string, Id> m_symbolNameToIdMap;
};

#endif	  // JAVA_PARSER_H
