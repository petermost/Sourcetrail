#include "FilePathFilter.h"

FilePathFilter::FilePathFilter(const std::string& filterString)
	: m_filterString(filterString), m_filterRegex(convertFilterStringToRegex(filterString))
{
}

std::string FilePathFilter::str() const
{
	return m_filterString;
}

bool FilePathFilter::isMatching(const FilePath& filePath) const
{
	return isMatching(filePath.str());
}

bool FilePathFilter::isMatching(const std::string& fileStr) const
{
	return std::regex_match(fileStr, m_filterRegex);
}

bool FilePathFilter::operator<(const FilePathFilter& other) const
{
	return m_filterString.compare(other.m_filterString) < 0;
}

std::regex FilePathFilter::convertFilterStringToRegex(const std::string& filterString)
{
	std::string regexFilterString = filterString;

	{
		std::regex regex("[\\\\/]");
		regexFilterString = std::regex_replace(regexFilterString, regex, "[\\\\/]");
	}

	{
		std::regex regex("([^\\\\])([^/])([\\]])");
		regexFilterString = std::regex_replace(regexFilterString, regex, "$1$2[\\]]");
	}

	{
		std::regex regex("([\\[])([^\\\\])");
		regexFilterString = std::regex_replace(regexFilterString, regex, "[\\[]$2");
	}

	{
		std::regex regex("[\\(]");
		regexFilterString = std::regex_replace(regexFilterString, regex, "[\\(]");
	}

	{
		std::regex regex("[\\)]");
		regexFilterString = std::regex_replace(regexFilterString, regex, "[\\)]");
	}

	{
		std::regex regex("[\\{]");
		regexFilterString = std::regex_replace(regexFilterString, regex, "[\\{]");
	}

	{
		std::regex regex("[\\}]");
		regexFilterString = std::regex_replace(regexFilterString, regex, "[\\}]");
	}

	{
		std::regex regex("[\\+]");
		regexFilterString = std::regex_replace(regexFilterString, regex, "[\\+]");
	}

	{
		std::regex regex("[\\-]");
		regexFilterString = std::regex_replace(regexFilterString, regex, "[\\-]");
	}

	{
		std::regex regex("[\\$]");
		regexFilterString = std::regex_replace(regexFilterString, regex, "[\\$]");
	}

	{
		std::regex regex("[\\.]");
		regexFilterString = std::regex_replace(regexFilterString, regex, "[\\.]");
	}

	{
		std::regex regex("[\\^]");
		regexFilterString = std::regex_replace(regexFilterString, regex, "[\\^]");
	}

	{
		std::regex regex("[\\*][\\*]");
		regexFilterString = std::regex_replace(regexFilterString, regex, ".{0,}");
	}

	{
		std::regex regex("[\\*]");
		regexFilterString = std::regex_replace(regexFilterString, regex, "[^\\\\/]*");
	}

	return std::regex(regexFilterString, std::regex::optimize);
}
