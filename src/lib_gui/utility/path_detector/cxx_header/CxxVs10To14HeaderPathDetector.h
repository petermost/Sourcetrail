#ifndef CXX_VS_10_TO_14_HEADER_PATH_DETECTOR_H
#define CXX_VS_10_TO_14_HEADER_PATH_DETECTOR_H

#include "Platform.h"
#include "PathDetector.h"

class CxxVs10To14HeaderPathDetector: public PathDetector
{
public:
	enum VisualStudioType
	{
		VISUAL_STUDIO_2010,
		VISUAL_STUDIO_2012,
		VISUAL_STUDIO_2013,
		VISUAL_STUDIO_2015
	};

	CxxVs10To14HeaderPathDetector(
		VisualStudioType type, bool isExpress, utility::Platform::Architecture architecture);

private:
	static int visualStudioTypeToVersion(const VisualStudioType t);
	static std::string visualStudioTypeToString(const VisualStudioType t);

	std::vector<FilePath> doGetPaths() const override;

	FilePath getVsInstallPathUsingRegistry() const;

	const int m_version;
	const bool m_isExpress;
	const utility::Platform::Architecture m_architecture;
};

#endif	  // CXX_VS_10_TO_14_HEADER_PATH_DETECTOR_H
