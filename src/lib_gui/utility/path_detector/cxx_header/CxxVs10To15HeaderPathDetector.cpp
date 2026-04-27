#include "CxxVs10To15HeaderPathDetector.h"

#include "FilePath.h"
#include "logging.h"
#include "utility.h"
#include "utilityCxxHeaderDetection.h"

#include <QSettings>
#include <QSysInfo>

#include <string>

using namespace utility;
using namespace std::string_literals;

using Architecture = Platform::Architecture;


static FilePath getVsInstallPathUsingRegistry(const LegacyVisualStudio::Version &version, bool isExpress, Architecture architecture)
{
	QString key = "HKEY_LOCAL_MACHINE\\SOFTWARE\\";
	if (architecture == Architecture::BITS_32)
	{
		key += "Wow6432Node\\";
	}
	key += "Microsoft\\";
	key += (isExpress ? QStringLiteral("VCExpress") : QStringLiteral("VisualStudio"));
	key += "\\" + version.number;

	QSettings expressKey(key, QSettings::NativeFormat);	  // NativeFormat means from Registry on Windows.
	QString value = expressKey.value("InstallDir").toString() + "../../";

	FilePath path(value.toStdString());
	if (path.exists())
	{
		LOG_INFO("Found working registry key for VS install path: " + key.toStdString());
		return path;
	}

	return FilePath();
}

CxxVs10To15HeaderPathDetector::CxxVs10To15HeaderPathDetector(const LegacyVisualStudio::Version &version, bool isExpress, Architecture architecture)
	: PathDetector(version.name + (isExpress ? " Express" : " ") + Platform::getArchitectureName(architecture))
	, m_version(version)
	, m_isExpress(isExpress)
	, m_architecture(architecture)
{
}


std::vector<FilePath> CxxVs10To15HeaderPathDetector::doGetPaths() const
{
	const FilePath vsInstallPath = getVsInstallPathUsingRegistry(m_version, m_isExpress, m_architecture);

	// vc++ headers
	std::vector<FilePath> headerSearchPaths;
	if (vsInstallPath.exists())
	{
		for (const std::string& subdirectory: {"vc/include"s, "vc/atlmfc/include"s})
		{
			FilePath headerSearchPath = vsInstallPath.getConcatenated(subdirectory);
			if (headerSearchPath.exists())
			{
				headerSearchPaths.push_back(headerSearchPath.makeCanonical());
			}
		}
	}

	if (!headerSearchPaths.empty())
	{
		utility::append(headerSearchPaths, utility::getWindowsSdkHeaderSearchPaths(m_architecture));
	}

	return headerSearchPaths;
}

