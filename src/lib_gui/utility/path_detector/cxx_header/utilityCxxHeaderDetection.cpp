#include "utilityCxxHeaderDetection.h"

#include "ToolChain.h"
#include "utility.h"
#include "utilityApp.h"
#include "utilityString.h"

#include <QSettings>
#include <QSysInfo>

#include <array>

using namespace std;
using namespace utility;

namespace utility
{

std::vector<std::string> getCxxHeaderPaths(const std::string& compilerName)
{
	std::vector<std::string> paths;

	const vector<string> arguments = {
		ClangCompiler::verboseOption(),
		ClangCompiler::languageOption(), ClangCompiler::CPP_LANGUAGE,
		ClangCompiler::preprocessOption(), "/dev/null"
	};

	const utility::ProcessOutput out = utility::executeProcess(compilerName, arguments);
	if (out.exitCode == 0)
	{
		std::string standardHeaders = utility::substrBetween(
			out.output, "#include <...> search starts here:\n", "\nEnd of search list");

		if (!standardHeaders.empty())
		{
			for (const std::string& s: utility::splitToVector(standardHeaders, '\n'))
			{
				paths.push_back(utility::trim(s));
			}
		}
	}

	return paths;
}

static std::string getWindowsSdkRegistryValue(const std::string &valueKey, Platform::Architecture architecture, const std::string &sdkVersion)
{
	QString key = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\");
	if (architecture == Platform::Architecture::BITS_32)
	{
		key += QStringLiteral("Wow6432Node\\");
	}
	key += QStringLiteral("Microsoft\\Microsoft SDKs\\Windows\\") + QString::fromStdString(sdkVersion);

	QSettings registrySettings(key, QSettings::NativeFormat); // NativeFormat means from Registry on Windows.
	QString value = registrySettings.value(QString::fromStdString(valueKey)).toString();

	return value.toStdString();
}

static FilePath getWindowsSdkInstallationFolder(Platform::Architecture architecture, const std::string &sdkVersion)
{
	return FilePath(getWindowsSdkRegistryValue("InstallationFolder"s, architecture, sdkVersion));
}

static string getWindowsSdkProductVersion(Platform::Architecture architecture, const std::string &sdkVersion)
{
	return getWindowsSdkRegistryValue("ProductVersion"s, architecture, sdkVersion) + ".0"s;
}

static std::vector<FilePath> getHeaderSearchPaths(const FilePath& sdkIncludePath)
{
	const array SUB_DIRECTORIES = {"shared"s, "um"s, "winrt"s, "ucrt"s};

	std::vector<FilePath> headerSearchPaths;

	bool usingSubdirectories = false;
	for (const std::string &subDirectory : SUB_DIRECTORIES)
	{
		const FilePath sdkSubdirectory = sdkIncludePath.getConcatenated(subDirectory);
		if (sdkSubdirectory.exists())
		{
			headerSearchPaths.push_back(sdkSubdirectory);
			usingSubdirectories = true;
		}
	}

	if (!usingSubdirectories)
	{
		headerSearchPaths.push_back(sdkIncludePath);
	}
	return headerSearchPaths;
}

std::vector<FilePath> getWindowsSdkHeaderSearchPaths(Platform::Architecture architecture)
{
	const vector<string> WINDOWS_SDK_VERSION = WindowsSdk::getVersions();

	// Look for unversioned include paths:

	std::vector<FilePath> unversionedSearchPaths;
	for (const std::string &windowsSdkVersion : WINDOWS_SDK_VERSION)
	{
		FilePath sdkPath = getWindowsSdkInstallationFolder(architecture, windowsSdkVersion);
		if (sdkPath.exists())
		{
			FilePath sdkIncludePath = sdkPath.getConcatenated("include"s);
			if (sdkIncludePath.exists())
			{
				unversionedSearchPaths = getHeaderSearchPaths(sdkIncludePath);
				break;
			}
		}
	}
   // Look for versioned include paths:

	std::vector<FilePath> versionedSearchPaths;
	for (const std::string &windowsSdkVersion : WINDOWS_SDK_VERSION)
	{
		FilePath sdkPath = getWindowsSdkInstallationFolder(architecture, windowsSdkVersion);
		if (sdkPath.exists())
		{
			std::string productVersion = getWindowsSdkProductVersion(architecture, windowsSdkVersion);
			FilePath versionedSdkIncludePath = sdkPath.getConcatenated("include"s).getConcatenated(productVersion);
			if (versionedSdkIncludePath.exists())
			{
				versionedSearchPaths = getHeaderSearchPaths(versionedSdkIncludePath);
				break;
			}
		}
	}

	/*
	// Look for *all* versioned include paths:

	std::vector<FilePath> versionedSearchPaths;
	for (const std::string &windowsSdkVersion : WINDOWS_SDK_VERSION)
	{
		FilePath sdkPath = getWindowsSdkInstallationFolder(architecture, windowsSdkVersion);
		if (sdkPath.exists())
		{
			for (const FilePath &versionedSdkIncludePath : FileSystem::getDirectSubDirectories(sdkPath.getConcatenated("include"s)))
			{
				versionedSearchPaths.append_range(getHeaderSearchPaths(versionedSdkIncludePath));
			}
		}
	}
	*/

	return concat(unversionedSearchPaths, versionedSearchPaths);
}

}	 // namespace utility
