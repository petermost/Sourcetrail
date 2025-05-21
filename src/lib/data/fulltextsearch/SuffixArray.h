#ifndef SUFFIX_ARRAY_H
#define SUFFIX_ARRAY_H

#include <string>
#include <vector>

class SuffixArray
{
public:
	SuffixArray(const std::string& text);

	std::vector<int> searchForTerm(const std::string& searchTerm) const;

private:
	std::vector<int> buildSuffixes() const;
	std::vector<int> buildLongestCommonPrefixes() const;

	const std::string m_text;
	const std::vector<int> m_suffixes;
	const std::vector<int> m_longestCommonPrefixes;
};

#endif	  // SUFFIX_ARRAY_H
