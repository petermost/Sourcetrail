#include "utilityString.h"

#include <boost/locale/boundary.hpp>
#include <boost/locale/conversion.hpp>
#include <boost/locale/encoding_utf.hpp>

#include <algorithm>
#include <cctype>
#include <iterator>
#include <string>

using namespace std;
using namespace boost::locale::boundary;

namespace
{

std::string doReplace(std::string str, const std::string& from, const std::string& to)
{
	size_t pos = 0;

	if (from.size() == 0)
	{
		return str;
	}

	while ((pos = str.find(from, pos)) != std::string::npos)
	{
		str.replace(pos, from.length(), to);
		pos += to.length();
	}

	return str;
}

std::string doReplaceBetween(const std::string& str, typename std::string::value_type startDelimiter, typename std::string::value_type endDelimiter, const std::string& to)
{
	size_t startPos = str.find(startDelimiter);
	if (startPos == std::string::npos)
	{
		return str;
	}

	size_t depth = 1;

	for (size_t pos = startPos + 1; pos < str.size(); pos++)
	{
		if (str[pos] == endDelimiter && depth)
		{
			depth--;

			if (depth == 0)
			{
				std::string end = doReplaceBetween(str.substr(pos + 1), startDelimiter, endDelimiter, to);
				return str.substr(0, startPos) + startDelimiter + to + endDelimiter + end;
			}
		}

		if (str[pos] == startDelimiter)
		{
			depth++;
		}
	}

	return str;
}

}	 // namespace

namespace utility
{


std::deque<std::string> split(const std::string& str, char delimiter)
{
	return split<std::deque<std::string>>(str, std::string(1, delimiter));
}

std::deque<std::string> split(const std::string& str, const std::string& delimiter)
{
	return split<std::deque<std::string>>(str, delimiter);
}

std::vector<std::string> splitToVector(const std::string& str, char delimiter)
{
	return split<std::vector<std::string>>(str, std::string(1, delimiter));
}

std::vector<std::string> splitToVector(const std::string& str, const std::string& delimiter)
{
	return split<std::vector<std::string>>(str, delimiter);
}

std::string join(const std::deque<std::string>& list, char delimiter)
{
	return join<std::deque<std::string>>(list, std::string(1, delimiter));
}

std::string join(const std::deque<std::string>& list, const std::string& delimiter)
{
	return join<std::deque<std::string>>(list, delimiter);
}

std::string join(const std::vector<std::string>& list, char delimiter)
{
	return join<std::vector<std::string>>(list, std::string(1, delimiter));
}

std::string join(const std::vector<std::string>& list, const std::string& delimiter)
{
	return join<std::vector<std::string>>(list, delimiter);
}

std::deque<std::string> tokenize(const std::string& str, char delimiter)
{
	return tokenize(str, std::string(1, delimiter));
}

std::deque<std::string> tokenize(const std::string& str, const std::string& delimiter)
{
	size_t pos = 0;
	size_t oldPos = 0;
	std::deque<std::string> c;

	do
	{
		pos = str.find(delimiter, oldPos);

		if (pos != oldPos)
		{
			c.push_back(str.substr(oldPos, pos - oldPos));
		}

		if (pos != std::string::npos)
		{
			c.push_back(str.substr(pos, delimiter.size()));
		}

		oldPos = pos + delimiter.size();
	} while (pos != std::string::npos && oldPos < str.size());

	return c;
}

std::deque<std::string> tokenize(const std::deque<std::string>& list, char delimiter)
{
	return tokenize(list, std::string(1, delimiter));
}

std::deque<std::string> tokenize(const std::deque<std::string>& list, const std::string& delimiter)
{
	std::deque<std::string> c;

	for (const std::string& str: list)
	{
		if (str.size())
		{
			std::deque<std::string> c2 = tokenize(str, delimiter);
			c.insert(c.end(), c2.begin(), c2.end());
		}
	}

	return c;
}

std::string substrBeforeFirst(const std::string& str, char delimiter)
{
	size_t pos = str.find(delimiter);
	if (pos != std::string::npos)
	{
		return str.substr(0, pos);
	}
	return str;
}

std::string substrBeforeFirst(const std::string& str, const std::string& delimiter)
{
	size_t pos = str.find(delimiter);
	if (pos != std::string::npos)
	{
		return str.substr(0, pos);
	}
	return str;
}

std::string substrBeforeLast(const std::string& str, char delimiter)
{
	size_t pos = str.rfind(delimiter);
	if (pos != std::string::npos)
	{
		return str.substr(0, pos);
	}
	return str;
}

std::string substrAfterLast(const std::string& str, char delimiter)
{
	size_t pos = str.rfind(delimiter);
	if (pos != std::string::npos)
	{
		return str.substr(pos + 1, std::string::npos);
	}
	return str;
}

std::string substrAfter(const std::string& str, char delimiter)
{
	size_t pos = str.find(delimiter);
	if (pos != std::string::npos)
	{
		return str.substr(pos + 1, std::string::npos);
	}
	return str;
}

std::string substrAfter(const std::string& str, const std::string& delimiter)
{
	size_t pos = str.find(delimiter);
	if (pos != std::string::npos)
	{
		return str.substr(pos + delimiter.size(), str.size());
	}
	return str;
}

std::string substrBetween(const std::string &str, const std::string &delimiter1, const std::string &delimiter2)
{
	size_t found_delimiter1 = str.find(delimiter1);
	found_delimiter1 += delimiter1.length();
	size_t found_delimiter2 = str.find(delimiter2, found_delimiter1);
	if (found_delimiter1 != str.npos && found_delimiter2 != str.npos)
	{
		return str.substr(found_delimiter1, found_delimiter2 - found_delimiter1);
	}
	return std::string();
}



bool isPrefix(const std::string &prefix, const std::string &text)
{
	if (prefix.size() <= text.size())
	{
		std::pair<typename std::string::const_iterator, typename std::string::const_iterator> res =
			std::mismatch(prefix.begin(), prefix.end(), text.begin());

		return res.first == prefix.end();
	}
	return false;
}

bool isPostfix(const std::string &postfix, const std::string &text)
{
	return text.size() >= postfix.size() && text.rfind(postfix) == (text.size() - postfix.size());
}


std::string replace(std::string str, const std::string& from, const std::string& to)
{
	return doReplace(str, from, to);
}

std::string replaceBetween(const std::string& str, char startDelimiter, char endDelimiter, const std::string& to)
{
	return doReplaceBetween(str, startDelimiter, endDelimiter, to);
}

std::string insertLineBreaksAtBlankSpaces(const std::string& s, size_t maxLineLength)
{
	const std::vector<std::string> atoms = splitToVector(s, " ");

	std::string ret;
	std::string currentLine;
	for (const std::string& atom: atoms)
	{
		if (currentLine.size() + 1 + atom.size() <= maxLineLength)
		{
			currentLine += " " + atom;
		}
		else
		{
			if (!ret.empty())
			{
				ret += "\n";
			}

			if (currentLine.empty())
			{
				ret += atom;
			}
			else
			{
				ret += currentLine;
				currentLine = atom;
			}
		}
	}
	if (!currentLine.empty())
	{
		if (!ret.empty())
		{
			ret += "\n";
		}
		ret += currentLine;
	}
	return ret;
}

std::string breakSignature(std::string signature, size_t maxLineLength, size_t tabWidth)
{
	if (signature.size() <= maxLineLength)
	{
		return signature;
	}

	size_t parenCount = 0;
	size_t parenPos = 0;
	size_t openParenPos = 0;
	size_t closeParenPos = 0;

	while (true)
	{
		closeParenPos = signature.find(')', openParenPos);
		openParenPos = signature.find('(', openParenPos);

		if (openParenPos == std::string::npos)
		{
			break;
		}
		else if (closeParenPos == std::string::npos)
		{
			return signature;
		}
		else if (closeParenPos < openParenPos)
		{
			if (parenCount == 0)
			{
				return signature;
			}
			parenCount--;
			openParenPos = closeParenPos;
		}
		else
		{
			if (!parenCount)
			{
				parenPos = openParenPos;
			}
			parenCount++;
		}

		openParenPos++;
	}

	if (!parenPos)
	{
		return signature;
	}

	std::string returnPart;
	std::string namePart = signature.substr(0, parenPos);
	std::string paramPart = signature.substr(parenPos);

	if (namePart.size() && namePart.back() == ' ')
	{
		namePart.pop_back();
	}

	size_t splitPos = std::string::npos;
	parenCount = 0;
	for (size_t i = namePart.size(); i > 0; i--)
	{
		const char c = namePart[i];
		if (c == '>' || c == ')')
		{
			parenCount++;
		}
		else if (c == '<' || c == '(')
		{
			parenCount--;
		}
		else if (!parenCount && c == ' ')
		{
			splitPos = i;
			break;
		}
	}

	if (splitPos != std::string::npos)
	{
		returnPart = namePart.substr(0, splitPos);
		namePart = namePart.substr(splitPos + 1);
	}

	return breakSignature(returnPart, namePart, paramPart, maxLineLength, tabWidth);
}

std::string breakSignature(std::string returnPart, std::string namePart, std::string paramPart, size_t maxLineLength, size_t tabWidth)
{
	namePart = ' ' + namePart;

	size_t totalSize = returnPart.size() + namePart.size() + paramPart.size();
	if (totalSize <= maxLineLength)
	{
		return returnPart + namePart + paramPart;
	}

	if (!paramPart.empty())
	{
		namePart += paramPart[0];
		paramPart.erase(0, 1);

		if (paramPart.front() == ' ')
		{
			paramPart.erase(0, 1);
		}
	}

	size_t parenPos = paramPart.rfind(')');
	std::string endPart;
	if (parenPos == 0)
	{
		namePart += paramPart;
		paramPart = "";
	}
	else if (parenPos != std::string::npos)
	{
		endPart = paramPart.substr(parenPos);
		paramPart = paramPart.substr(0, parenPos);
	}

	if (!paramPart.empty() && paramPart.size() + tabWidth - endPart.size() > maxLineLength)
	{
		std::vector<std::string> paramLines;
		while (true)
		{
			size_t parenCount = 0;
			bool split = false;
			for (size_t i = 0; i < paramPart.size(); i++)
			{
				const char c = paramPart[i];
				if (parenCount == 0 && c == ',')
				{
					paramLines.push_back(paramPart.substr(0, i + 1));
					paramPart = paramPart.substr(i + 2);
					split = true;
					break;
				}
				else if (c == '<' || c == '(')
				{
					parenCount++;
				}
				else if (c == '>' || c == ')')
				{
					parenCount--;
				}
			}

			if (!split)
			{
				paramLines.push_back(paramPart);
				break;
			}
		}

		paramPart = "";
		for (const std::string& str: paramLines)
		{
			paramPart += "\n\t" + str;
			const size_t length = tabWidth + str.size();
			maxLineLength = std::max(length, maxLineLength);
		}
	}
	else if (!paramPart.empty())
	{
		paramPart = "\n\t" + paramPart;
	}

	if (returnPart.size() + namePart.size() <= maxLineLength)
	{
		namePart = returnPart + namePart;
		returnPart = "";
	}

	std::string sig;

	if (!returnPart.empty())
	{
		sig += returnPart + '\n';
	}

	sig += namePart;

	if (!paramPart.empty())
	{
		sig += paramPart;
	}

	if (!endPart.empty())
	{
		sig += '\n' + endPart;
	}

	return sig;
}

std::string trim(const std::string& str)
{
	auto wsfront = std::find_if_not(str.begin(), str.end(), [](int c) { return std::isspace(c); });
	auto wsback = std::find_if_not(str.rbegin(), str.rend(), [](int c) { return std::isspace(c); }).base();
	return (wsback <= wsfront ? std::string() : std::string(wsfront, wsback));
}

std::string elide(const std::string& str, ElideMode mode, size_t size)
{
	if (str.size() <= size || str.size() <= 3)
	{
		return str;
	}

	switch (mode)
	{
	case ELIDE_LEFT:
		return "..." + str.substr(str.size() - size - 3, str.size());
	case ELIDE_MIDDLE:
		return str.substr(0, size / 2 - 1) + "..." +
			str.substr(str.size() - (size / 2 - 2), str.size());
	case ELIDE_RIGHT:
		return str.substr(0, size - 3) + "...";
	}

	return "";
}

std::string convertWhiteSpacesToSingleSpaces(const std::string& str)
{
	std::string res = replace(str, "\n", " ");
	res = replace(res, "\t", " ");

	std::deque<std::string> parts = split<std::deque<std::string>>(res, " ");
	for (size_t i = 1; i <= parts.size(); i++)
	{
		if (!parts[i - 1].size())
		{
			parts.erase(parts.begin() + i - 1);
			i--;
		}
	}

	return join<std::deque<std::string>>(parts, " ");
}

///////////////////////////////////////////////////////////////////////////////
//
//
// Locale specific functions:
//
///////////////////////////////////////////////////////////////////////////////

std::string encodeToUtf8String(const std::wstring &s)
{
	return boost::locale::conv::utf_to_utf<char>(s);
}

std::wstring decodeFromUtf8String(const std::string &s)
{
	return boost::locale::conv::utf_to_utf<wchar_t>(s);
}


std::string toUpperCase(const std::string& in)
{
    return boost::locale::to_upper(in);
}

std::string toLowerCase(const std::string& in)
{
    return boost::locale::to_lower(in);
}

wstring toUpperCase(const wstring &s)
{
	return boost::locale::to_upper(s);
}

wstring toLowerCase(const wstring &s)
{
	return boost::locale::to_lower(s);
}

bool equalsCaseInsensitive(const std::string &a, const std::string &b)
{
	return toLowerCase(a) == toLowerCase(b);
}

bool caseInsensitiveLess(const std::string& s1, const std::string& s2)
{
	return toLowerCase(s1) < toLowerCase(s2);
}

vector<string> splitToCharacters(const string &s)
{
	vector<string> chars;

	ssegment_index characterMap(character, s.begin(), s.end());
	for (auto it = characterMap.begin(); it != characterMap.end(); ++it)
		chars.push_back(*it);

	return chars;
}



}	 // namespace utility
