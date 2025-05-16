#ifndef CXX_VS_15_TO_LATEST_HEADER_PATH_DETECTOR_H
#define CXX_VS_15_TO_LATEST_HEADER_PATH_DETECTOR_H

#include "PathDetector.h"

class CxxVs15ToLatestHeaderPathDetector: public PathDetector
{
public:
	CxxVs15ToLatestHeaderPathDetector(const std::string &versionRange);

private:
	const std::string m_versionRange;

	std::vector<FilePath> doGetPaths() const override;

	static std::string getVsWhereProperty(const std::string &versionRange, const std::string &propertyName);
};

#endif
