#include "CxxVs10To14HeaderPathDetector.h"

#include <string>

#include <QSettings>
#include <QSysInfo>

#include "FilePath.h"
#include "logging.h"
#include "utility.h"
#include "utilityCxxHeaderDetection.h"

using namespace utility;
using namespace std::string_literals;

CxxVs10To14HeaderPathDetector::CxxVs10To14HeaderPathDetector(VisualStudioType type, bool isExpress, Platform::Architecture architecture)
	: PathDetector(visualStudioTypeToString(type) + (isExpress ? " Express" : " ") + Platform::getArchitectureName(architecture))
	, m_version(visualStudioTypeToVersion(type))
	, m_isExpress(isExpress)
	, m_architecture(architecture)
{
}

int CxxVs10To14HeaderPathDetector::visualStudioTypeToVersion(const VisualStudioType t)
{
	switch (t)
	{
	case VISUAL_STUDIO_2010:
		return 10;
	case VISUAL_STUDIO_2012:
		return 11;
	case VISUAL_STUDIO_2013:
		return 12;
	case VISUAL_STUDIO_2015:
		return 14;
	}
	return 0;
}

std::string CxxVs10To14HeaderPathDetector::visualStudioTypeToString(const VisualStudioType t)
{
	std::string ret = "Visual Studio";
	switch (t)
	{
	case VISUAL_STUDIO_2010:
		return ret + " 2010";
	case VISUAL_STUDIO_2012:
		return ret + " 2012";
	case VISUAL_STUDIO_2013:
		return ret + " 2013";
	case VISUAL_STUDIO_2015:
		return ret + " 2015";
	}
	return ret;
}

std::vector<FilePath> CxxVs10To14HeaderPathDetector::doGetPaths() const
{
	const FilePath vsInstallPath = getVsInstallPathUsingRegistry();

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

FilePath CxxVs10To14HeaderPathDetector::getVsInstallPathUsingRegistry() const
{
	QString key = "HKEY_LOCAL_MACHINE\\SOFTWARE\\";
	if (m_architecture == Platform::Architecture::BITS_32)
	{
		key += "Wow6432Node\\";
	}
	key += "Microsoft\\";
	key += (m_isExpress ? QStringLiteral("VCExpress") : QStringLiteral("VisualStudio"));
	key += "\\" + QString::number(m_version) + ".0";

	QSettings expressKey(
		key, QSettings::NativeFormat);	  // NativeFormat means from Registry on Windows.
	QString value = expressKey.value("InstallDir").toString() + "../../";

	FilePath path(value.toStdString());
	if (path.exists())
	{
		LOG_INFO("Found working registry key for VS install path: " + key.toStdString());
		return path;
	}

	return FilePath();
}
