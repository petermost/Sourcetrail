#ifndef SOURCE_GROUP_TYPE_H
#define SOURCE_GROUP_TYPE_H

#include "language_packages.h"

#include <QMetaType>

#include <string>

enum class SourceGroupType
{
	UNKNOWN,
#if BUILD_CXX_LANGUAGE_PACKAGE
	C_EMPTY,
	CXX_EMPTY,
	CXX_CDB,
	CXX_CODEBLOCKS,
	CXX_VS,
#endif	  // BUILD_CXX_LANGUAGE_PACKAGE
#if BUILD_JAVA_LANGUAGE_PACKAGE
	JAVA_EMPTY,
	JAVA_MAVEN,
	JAVA_GRADLE,
#endif	  // BUILD_JAVA_LANGUAGE_PACKAGE
	CUSTOM_COMMAND
};

Q_DECLARE_METATYPE(SourceGroupType)

std::string sourceGroupTypeToString(SourceGroupType v);
std::string sourceGroupTypeToProjectSetupString(SourceGroupType v);
SourceGroupType stringToSourceGroupType(const std::string& v);

#endif	  // SOURCE_GROUP_TYPE_H
