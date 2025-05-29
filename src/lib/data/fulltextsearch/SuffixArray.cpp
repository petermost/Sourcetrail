#include "SuffixArray.h"

#include "utilityString.h"

#include <algorithm>

using namespace utility;

namespace
{

struct Suffix
{
	int index;
	int rank[2];

	static int cmp(const Suffix &a, const Suffix &b)
	{
		return (a.rank[0] == b.rank[0]) ? (a.rank[1] < b.rank[1] ? 1 : 0)
										: (a.rank[0] < b.rank[0] ? 1 : 0);
	}
};


}

SuffixArray::SuffixArray(const std::string& text)
	: m_text32(convertToUtf32(toLowerCase(text)))
	, m_suffixes(buildSuffixes())
	, m_longestCommonPrefixes(buildLongestCommonPrefixes())
{
}

std::vector<int> SuffixArray::searchForTerm(const std::string& searchTerm) const
{
	const std::u32string searchTerm32 = convertToUtf32(toLowerCase(searchTerm));

	const int termLength = static_cast<int>(searchTerm32.length());
	const int textLength = static_cast<int>(m_text32.length());
	int l = -1;
	int r = textLength;
	int m;

	std::vector<int> matches;
	int compareResult;
	while (l + 1 < r)
	{
		m = (l + r + 1) / 2;
		compareResult = searchTerm32.compare(m_text32.substr(m_suffixes[m], termLength));
		if (compareResult < 0)
		{
			r = m;
		}
		else if (compareResult > 0)
		{
			l = m;
		}
		else
		{
			matches.push_back(m_suffixes[m]);
			for (int lower = m - 1; lower >= 0 && m_longestCommonPrefixes[lower] >= termLength; lower--)
			{
				matches.push_back(m_suffixes[lower]);
			}
			for (int higher = m + 1; higher < textLength && m_longestCommonPrefixes[higher - 1] >= termLength; higher++)
			{
				matches.push_back(m_suffixes[higher]);
			}
			break;
		}
	}

	std::sort(matches.begin(), matches.end());

	return matches;
}



std::vector<int> SuffixArray::buildSuffixes() const
{
	const int n = static_cast<int>(m_text32.length());
	std::vector<Suffix> suffixes;
	suffixes.reserve(n);

	Suffix s;
	for (int i = 0; i < n; i++)
	{
		s.index = i;
		s.rank[0] = m_text32[i];
		s.rank[1] = ((i + 1) < n) ? (m_text32[i + 1]) : -1;
		suffixes.push_back(s);
	}

	std::sort(suffixes.begin(), suffixes.end(), Suffix::cmp);

	std::vector<int> ind(n, 0);
	for (int k = 4; k < 2 * n; k = k * 2)
	{
		int rank = 0;
		int prev_rank = suffixes[0].rank[0];
		suffixes[0].rank[0] = rank;
		ind[suffixes[0].index] = 0;

		for (int i = 1; i < n; i++)
		{
			if (suffixes[i].rank[0] == prev_rank && suffixes[i].rank[1] == suffixes[i - 1].rank[1])
			{
				prev_rank = suffixes[i].rank[0];
				suffixes[i].rank[0] = rank;
			}
			else
			{
				prev_rank = suffixes[i].rank[0];
				suffixes[i].rank[0] = ++rank;
			}
			ind[suffixes[i].index] = i;
		}

		for (int i = 0; i < n; i++)
		{
			int nextindex = suffixes[i].index + k / 2;
			suffixes[i].rank[1] = (nextindex < n) ? suffixes[ind[nextindex]].rank[0] : -1;
		}

		std::sort(suffixes.begin(), suffixes.end(), Suffix::cmp);
	}

	std::vector<int> suffixArr;
	for (int i = 0; i < n; i++)
	{
		suffixArr.push_back(suffixes[i].index);
	}

	return suffixArr;
}

std::vector<int> SuffixArray::buildLongestCommonPrefixes() const
{
	const int n = static_cast<int>(m_suffixes.size());

	std::vector<int> lcp(n, 0);
	std::vector<int> invSuff(n, 0);

	for (int i = 0; i < n; i++)
	{
		invSuff[m_suffixes[i]] = i;
	}

	int k = 0;

	for (int i = 0; i < n; i++)
	{
		if (invSuff[i] == n - 1)
		{
			k = 0;
			continue;
		}

		int j = m_suffixes[invSuff[i] + 1];

		while (i + k < n && j + k < n && m_text32[i + k] == m_text32[j + k])
		{
			k++;
		}

		lcp[invSuff[i]] = k;

		if (k > 0)
		{
			k--;
		}
	}

	return lcp;
}

// template <typename T>
// void printArr(const std::vector<T> &arr)
// {
// 	for (size_t i = 0; i < arr.size(); i++)
// 	{
// 		std::cout << arr[i] << " ";
// 	}
// 	std::cout << std::endl;
// }

// void SuffixArray::printArray() const
// {
// 	std::cout << "Suffix Array : \n";
// 	printArr(m_suffixArray);
// 	for (size_t i = 0; i < m_suffixArray.size(); i++)
// 	{
// 		std::string suffix = m_text.substr(m_suffixArray[i]);
// 		std::cout << i << ": \"" << suffix << "\"" << std::endl;
// 	}
// }

// void SuffixArray::printLCP() const
// {
// 	std::cout << "\nLCP Array : \n";
// 	printArr(m_longestCommonPrefix);
// 	for (size_t i = 0; i < m_suffixArray.size(); i++)
// 	{
// 		std::string prefix = m_text.substr(m_suffixArray[i], m_longestCommonPrefix[i]);
// 		std::cout << i << ": \"" << prefix << "\"" << std::endl;
// 	}
// }
