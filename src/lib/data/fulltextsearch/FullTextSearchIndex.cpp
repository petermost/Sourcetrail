#include "FullTextSearchIndex.h"
#include <limits>

#include "logging.h"
#include "tracing.h"

void FullTextSearchIndex::addFile(Id fileId, const std::string& fileContent)
{
	if (fileContent.empty())
	{
		LOG_ERROR("empty file not added to fulltextsearch index");
	}

	if (static_cast<int>(fileContent.size()) >= std::numeric_limits<int>::max())
	{
		LOG_ERROR("file too big not added to fulltextsearch index");
	}

	FullTextSearchFile fts_file(fileId, SuffixArray(fileContent));

	m_files.access()->push_back(fts_file);
}

std::vector<FullTextSearchResult> FullTextSearchIndex::searchForTerm(const std::string& term) const
{
	TRACE();

	std::vector<FullTextSearchResult> ret;
	aidkit::access([&](const auto &files)
	{
		for (const FullTextSearchFile &fullTextSearchFile : files)
		{
			FullTextSearchResult hit;
			hit.fileId = fullTextSearchFile.fileId;
			hit.positions = fullTextSearchFile.array.searchForTerm(term);
			std::sort(hit.positions.begin(), hit.positions.end());
			if (!hit.positions.empty())
			{
				ret.push_back(hit);
			}
		}
	}, m_files);

	return ret;
}

size_t FullTextSearchIndex::fileCount() const
{
	return m_files.access()->size();
}

void FullTextSearchIndex::clear()
{
	m_files.access()->clear();
}
