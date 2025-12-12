#include "utilityCxxHeaderDetection.h"

#include <QSettings>
#include <QSysInfo>

#include "FileSystem.h"
#include "ToolChain.h"
#include "utilityApp.h"
#include "utilityString.h"

using namespace std;

namespace utility
{

namespace
{
FilePath getWindowsSdkRootPathUsingRegistry(Platform::Architecture architecture, const std::string& sdkVersion)
{
	QString key = QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\");
	if (architecture == Platform::Architecture::BITS_32)
	{
		key += QStringLiteral("Wow6432Node\\");
	}
	key += QStringLiteral("Microsoft\\Microsoft SDKs\\Windows\\") + sdkVersion.c_str();

	QSettings expressKey(key, QSettings::NativeFormat);	  // NativeFormat means from Registry on Windows.
	QString value = expressKey.value(QStringLiteral("InstallationFolder")).toString();

	FilePath path(value.toStdString());
	if (path.exists())
	{
		return path;
	}
	return FilePath();
}

}

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

std::vector<FilePath> getWindowsSdkHeaderSearchPaths(Platform::Architecture architecture)
{
	std::vector<FilePath> headerSearchPaths;

	std::vector<std::string> windowsSdkVersions = WindowsSdk::getVersions();

	for (size_t i = 0; i < windowsSdkVersions.size(); i++)
	{
		const FilePath sdkPath = getWindowsSdkRootPathUsingRegistry(architecture, windowsSdkVersions[i]);
		if (sdkPath.exists())
		{
			const FilePath sdkIncludePath = sdkPath.getConcatenated("include/");
			if (sdkIncludePath.exists())
			{
				bool usingSubdirectories = false;
				for (const std::string &subDirectory : {"shared"s, "um"s, "winrt"s})
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
				break;
			}
		}
	}
	{
		const FilePath sdkPath = getWindowsSdkRootPathUsingRegistry(architecture, "v10.0");
		if (sdkPath.exists())
		{
			for (const FilePath &versionPath : FileSystem::getDirectSubDirectories(sdkPath.getConcatenated("include/")))
			{
				const FilePath ucrtPath = versionPath.getConcatenated("ucrt");
				if (ucrtPath.exists())
				{
					headerSearchPaths.push_back(ucrtPath);
					break;
				}
			}
		}
	}

	return headerSearchPaths;
}

}	 // namespace utility
