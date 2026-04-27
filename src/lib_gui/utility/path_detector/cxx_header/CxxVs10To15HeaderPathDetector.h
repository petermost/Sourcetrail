#ifndef CXX_VS_10_TO_15_HEADER_PATH_DETECTOR_H
#define CXX_VS_10_TO_15_HEADER_PATH_DETECTOR_H

#include "Platform.h"
#include "PathDetector.h"
#include "ToolChain.h"

class CxxVs10To15HeaderPathDetector: public PathDetector
{
public:
	CxxVs10To15HeaderPathDetector(const LegacyVisualStudio::Version &version, bool isExpress, utility::Platform::Architecture architecture);

private:
	std::vector<FilePath> doGetPaths() const override;

	const LegacyVisualStudio::Version m_version;
	const bool m_isExpress;
	const utility::Platform::Architecture m_architecture;
};

#endif
