#ifndef NAME_DELIMITER_TYPE_H
#define NAME_DELIMITER_TYPE_H

#include <string>

enum class NameDelimiterType
{
	UNKNOWN,
	FILE,
	CXX,
	JAVA,
	MODULE
};

std::string nameDelimiterTypeToString(NameDelimiterType delimiter);
NameDelimiterType stringToNameDelimiterType(const std::string& s);

#endif	  // NAME_DELIMITER_TYPE_H
