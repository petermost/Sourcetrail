#ifndef UTILITY_STRING_H
#define UTILITY_STRING_H

#include <deque>
#include <sstream>
#include <string>
#include <vector>

namespace utility
{


std::deque<std::string> split(const std::string& str, char delimiter);
std::deque<std::string> split(const std::string& str, const std::string& delimiter);
std::vector<std::string> splitToVector(const std::string& str, char delimiter);
std::vector<std::string> splitToVector(const std::string& str, const std::string& delimiter);

std::string join(const std::deque<std::string>& list, char delimiter);
std::string join(const std::deque<std::string>& list, const std::string& delimiter);
std::string join(const std::vector<std::string>& list, char delimiter);
std::string join(const std::vector<std::string>& list, const std::string& delimiter);

std::deque<std::string> tokenize(const std::string& str, char delimiter);
std::deque<std::string> tokenize(const std::string& str, const std::string& delimiter);
std::deque<std::string> tokenize(const std::deque<std::string>& list, char delimiter);
std::deque<std::string> tokenize(const std::deque<std::string>& list, const std::string& delimiter);

std::string substrBeforeFirst(const std::string& str, char delimiter);
std::string substrBeforeFirst(const std::string& str, const std::string& delimiter);
std::string substrBeforeLast(const std::string& str, char delimiter);
std::string substrAfterLast(const std::string& str, char delimiter);
std::string substrAfter(const std::string& str, char delimiter);
std::string substrAfter(const std::string& str, const std::string& delimiter);
std::string substrBetween(const std::string& str, const std::string& delimiter1, const std::string& delimiter2);

bool isPrefix(const std::string& prefix, const std::string& text);
bool isPostfix(const std::string& postfix, const std::string& text);

std::string replace(std::string str, const std::string& from, const std::string& to);
std::string replaceBetween(const std::string& str, char startDelimiter, char endDelimiter, const std::string& to);

std::string insertLineBreaksAtBlankSpaces(const std::string& s, size_t maxLineLength);
std::string breakSignature(std::string signature, size_t maxLineLength, size_t tabWidth);
std::string breakSignature(std::string returnPart, std::string namePart, std::string paramPart, size_t maxLineLength, size_t tabWidth);

std::string trim(const std::string& str);

enum ElideMode
{
	ELIDE_LEFT,
	ELIDE_MIDDLE,
	ELIDE_RIGHT
};

std::string elide(const std::string& str, ElideMode mode, size_t size);

std::string convertWhiteSpacesToSingleSpaces(const std::string& str);


template <typename ContainerType>
ContainerType split(const std::string& str, const std::string& delimiter)
{
	size_t pos = 0;
	size_t oldPos = 0;
	ContainerType c;

	do
	{
		pos = str.find(delimiter, oldPos);
		c.push_back(str.substr(oldPos, pos - oldPos));
		oldPos = pos + delimiter.size();
	} while (pos < str.length());

	return c;
}

template <typename ContainerType>
std::string join(const ContainerType& list, const std::string& delimiter)
{
	std::stringstream ss;
	bool first = true;
	for (const std::string& str: list)
	{
		if (!first)
		{
			ss << delimiter;
		}
		first = false;

		ss << str;
	}
	return ss.str();
}

//
// Locale specific functions:
//

// encodeToUtf8/decodeFromUtf8 are no longer needed, but we keep it for backward
// compatibility and in case of bugs so we can check where these conversions were done:
inline const std::string &encodeToUtf8(const std::string& s) { return s; }
inline const std::string &decodeFromUtf8(const std::string& s) { return s; }

std::u32string convertToUtf32(const std::string &utf8chars);
std::string convertToUtf8(const std::u32string &utf32chars);

std::wstring encodeToWString(const std::string &s);
std::string decodeFromWString(const std::wstring &s);

std::string toLowerCase(const std::string& in);

std::u32string toLowerCase(const std::u32string &in);

bool equalsCaseInsensitive(const std::string& a, const std::string& b);

bool caseInsensitiveLess(const std::string& s1, const std::string& s2);

}	 // namespace utility

#endif	  // UTILITY_STRING_H
