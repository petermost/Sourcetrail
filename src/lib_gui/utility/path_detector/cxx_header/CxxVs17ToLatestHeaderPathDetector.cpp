#include "CxxVs17ToLatestHeaderPathDetector.h"

#include "FilePath.h"
#include "TextAccess.h"
#include "utility.h"
#include "utilityApp.h"
#include "utilityCxxHeaderDetection.h"
#include "utilityString.h"

#include <string>

using namespace std;
using namespace utility;
using namespace boost::chrono;
using namespace string_literals;

static std::string getVsWhereProperty(const std::string &versionRange, const std::string &propertyName)
{
	std::string propertyValue;

	// vswhere location: https://github.com/microsoft/vswhere#visual-studio-locator

	const std::vector<FilePath> expandedPaths = FilePath("%ProgramFiles(x86)%/Microsoft Visual Studio/Installer/vswhere.exe").expandEnvironmentVariables();
	if (!expandedPaths.empty())
	{
		const utility::ProcessOutput out = utility::executeProcess(expandedPaths.front().str(), {
			"-version", versionRange,
			"-property", propertyName
			}, FilePath(), false, milliseconds(10'000));

		if (out.exitCode == 0)
			propertyValue = out.output;
	}
	return propertyValue;
}

static string getDefaultToolsVersion(const FilePath &vsInstallPath)
{
	string defaultToolsVersion;

	// https://devblogs.microsoft.com/cppblog/compiler-tools-layout-in-visual-studio-15/#finding-the-default-msvc-tools

	FilePath toolsVersionPath = vsInstallPath.getConcatenated("VC/Auxiliary/Build/Microsoft.VCToolsVersion.default.txt");
	if (toolsVersionPath.exists())
	{
		shared_ptr<TextAccess> toolsVersionText = TextAccess::createFromFile(toolsVersionPath);
		defaultToolsVersion = trim(toolsVersionText->getLine(1));
	}
	return defaultToolsVersion;
}

CxxVs17ToLatestHeaderPathDetector::CxxVs17ToLatestHeaderPathDetector(const string &versionRange)
	: PathDetector(getVsWhereProperty(versionRange, "displayName"))
	, m_versionRange(versionRange)
{
}

std::vector<FilePath> CxxVs17ToLatestHeaderPathDetector::doGetPaths() const
{
	std::vector<FilePath> headerSearchPaths;

	const FilePath vsInstallPath = getVsWhereProperty(m_versionRange, "installationPath");
	if (vsInstallPath.exists())
	{
		// Look for versioned include paths:

		string defaultToolsVersion = getDefaultToolsVersion(vsInstallPath);
		if (!defaultToolsVersion.empty())
		{
			FilePath versionedIncludePath = vsInstallPath.getConcatenated("VC/Tools/MSVC").getConcatenated(defaultToolsVersion);
			if (versionedIncludePath.exists())
			{
				headerSearchPaths.push_back(versionedIncludePath.getConcatenated("include"));
				headerSearchPaths.push_back(versionedIncludePath.getConcatenated("atlmfc/include"));
			}
		}
		/*
		// Look for *all* versioned include paths:

		for (const FilePath& versionPath: FileSystem::getDirectSubDirectories(vsInstallPath.getConcatenated("VC/Tools/MSVC")))
		{
			if (versionPath.exists())
			{
				headerSearchPaths.push_back(versionPath.getConcatenated("include"));
				headerSearchPaths.push_back(versionPath.getConcatenated("atlmfc/include"));
			}
		}
		*/
		headerSearchPaths.push_back(vsInstallPath.getConcatenated("VC/Auxiliary/VS/include"));
		headerSearchPaths.push_back(vsInstallPath.getConcatenated("VC/Auxiliary/VS/UnitTest/include"));
	}

	if (!headerSearchPaths.empty())
	{
		std::vector<FilePath> windowsSdkHeaderSearchPaths = utility::getWindowsSdkHeaderSearchPaths(Platform::Architecture::BITS_32);
		if (windowsSdkHeaderSearchPaths.empty())
		{
			windowsSdkHeaderSearchPaths = utility::getWindowsSdkHeaderSearchPaths(Platform::Architecture::BITS_64);
		}
		append(headerSearchPaths, windowsSdkHeaderSearchPaths);
	}
	return headerSearchPaths;
}
