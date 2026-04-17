#include "NameDelimiterType.h"

std::string nameDelimiterTypeToString(NameDelimiterType delimiter)
{
	switch (delimiter)
	{
		case NameDelimiterType::FILE:
			return "/";
		case NameDelimiterType::CXX:
			return "::";
		case NameDelimiterType::JAVA:
			return ".";
		case NameDelimiterType::MODULE:
			return ".";
		default:
			break;
	}
	return "@";
}

NameDelimiterType stringToNameDelimiterType(const std::string &s)
{
	if (s == nameDelimiterTypeToString(NameDelimiterType::FILE))
	{
		return NameDelimiterType::FILE;
	}
	if (s == nameDelimiterTypeToString(NameDelimiterType::CXX))
	{
		return NameDelimiterType::CXX;
	}
	if (s == nameDelimiterTypeToString(NameDelimiterType::JAVA))
	{
		return NameDelimiterType::JAVA;
	}
	if (s == nameDelimiterTypeToString(NameDelimiterType::MODULE))
	{
		return NameDelimiterType::MODULE;
	}
	return NameDelimiterType::UNKNOWN;
}
