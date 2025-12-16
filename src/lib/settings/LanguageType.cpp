#include "LanguageType.h"

std::string languageTypeToString(LanguageType t)
{
	switch (t)
	{
#if BUILD_CXX_LANGUAGE_PACKAGE
	case LanguageType::C:
		return "C";
	case LanguageType::CXX:
		return "C++";
#endif	  // BUILD_CXX_LANGUAGE_PACKAGE
#if BUILD_JAVA_LANGUAGE_PACKAGE
	case LanguageType::JAVA:
		return "Java";
#endif	  // BUILD_JAVA_LANGUAGE_PACKAGE
	case LanguageType::CUSTOM:
		return "Custom";
	case LanguageType::UNKNOWN:
		break;
	}
	return "unknown";
}

LanguageType getLanguageTypeForSourceGroupType(SourceGroupType t)
{
	switch (t)
	{
#if BUILD_CXX_LANGUAGE_PACKAGE
	case SourceGroupType::C_EMPTY:
		return LanguageType::C;
	case SourceGroupType::CXX_EMPTY:
		return LanguageType::CXX;
	case SourceGroupType::CXX_CDB:
		return LanguageType::CXX;
	case SourceGroupType::CXX_CODEBLOCKS:
		return LanguageType::CXX;
	case SourceGroupType::CXX_VS:
		return LanguageType::CXX;
#endif	  // BUILD_CXX_LANGUAGE_PACKAGE
#if BUILD_JAVA_LANGUAGE_PACKAGE
	case SourceGroupType::JAVA_EMPTY:
		return LanguageType::JAVA;
	case SourceGroupType::JAVA_MAVEN:
		return LanguageType::JAVA;
	case SourceGroupType::JAVA_GRADLE:
		return LanguageType::JAVA;
#endif	  // BUILD_JAVA_LANGUAGE_PACKAGE
	case SourceGroupType::CUSTOM_COMMAND:
		return LanguageType::CUSTOM;
	default:
		break;
	}

	return LanguageType::UNKNOWN;
}
