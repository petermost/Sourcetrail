#ifndef CXX_VS_17_TO_LATEST_HEADER_PATH_DETECTOR_H
#define CXX_VS_17_TO_LATEST_HEADER_PATH_DETECTOR_H

#include "PathDetector.h"

class CxxVs17ToLatestHeaderPathDetector: public PathDetector
{
public:
	CxxVs17ToLatestHeaderPathDetector(const std::string &versionRange);

private:
	const std::string m_versionRange;

	std::vector<FilePath> doGetPaths() const override;
};

#endif
