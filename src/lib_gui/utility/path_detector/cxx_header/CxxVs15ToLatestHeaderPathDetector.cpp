#include "CxxVs15ToLatestHeaderPathDetector.h"

#include <string>

#include "FilePath.h"
#include "FileSystem.h"
#include "utility.h"
#include "utilityApp.h"
#include "utilityCxxHeaderDetection.h"

using namespace std;
using namespace utility;
using namespace boost::chrono;

CxxVs15ToLatestHeaderPathDetector::CxxVs15ToLatestHeaderPathDetector(const string &versionRange)
	: PathDetector(getVsWhereProperty(versionRange, "displayName"))
	, m_versionRange(versionRange)
{
}

std::vector<FilePath> CxxVs15ToLatestHeaderPathDetector::doGetPaths() const
{
	std::vector<FilePath> headerSearchPaths;

	{
		const std::string installationPath = getVsWhereProperty(m_versionRange, "installationPath");

		if (!installationPath.empty())
		{
			const FilePath vsInstallPath(installationPath);
			if (vsInstallPath.exists())
			{
				for (const FilePath& versionPath: FileSystem::getDirectSubDirectories(
							vsInstallPath.getConcatenated("VC/Tools/MSVC")))
				{
					if (versionPath.exists())
					{
						headerSearchPaths.push_back(versionPath.getConcatenated("include"));
						headerSearchPaths.push_back(
							versionPath.getConcatenated("atlmfc/include"));
					}
				}
				headerSearchPaths.push_back(
					vsInstallPath.getConcatenated("VC/Auxiliary/VS/include"));
				headerSearchPaths.push_back(
					vsInstallPath.getConcatenated("VC/Auxiliary/VS/UnitTest/include"));
			}
		}
	}

	if (!headerSearchPaths.empty())
	{
		std::vector<FilePath> windowsSdkHeaderSearchPaths = utility::getWindowsSdkHeaderSearchPaths(
			Platform::Architecture::BITS_32);
		if (windowsSdkHeaderSearchPaths.empty())
		{
			windowsSdkHeaderSearchPaths = utility::getWindowsSdkHeaderSearchPaths(
				Platform::Architecture::BITS_64);
		}
		utility::append(headerSearchPaths, windowsSdkHeaderSearchPaths);
	}

	return headerSearchPaths;
}

std::string CxxVs15ToLatestHeaderPathDetector::getVsWhereProperty(const std::string &versionRange, const std::string &propertyName)
{
	std::string propertyValue;

	// vswhere location: https://github.com/microsoft/vswhere#visual-studio-locator

	const std::vector<FilePath> expandedPaths =
		FilePath("%ProgramFiles(x86)%/Microsoft Visual Studio/Installer/vswhere.exe")
			.expandEnvironmentVariables();
	if (!expandedPaths.empty())
	{
		const utility::ProcessOutput out = utility::executeProcess(expandedPaths.front().str(),
			{ "-version", versionRange, "-property", propertyName },
			FilePath(), false, milliseconds(10000));

		if (out.exitCode == 0)
			propertyValue = out.output;
	}
	return propertyValue;
}

