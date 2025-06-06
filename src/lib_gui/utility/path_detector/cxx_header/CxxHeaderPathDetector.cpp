#include "CxxHeaderPathDetector.h"

#include "FilePath.h"
#include "utilityCxxHeaderDetection.h"
#include "utilityString.h"

CxxHeaderPathDetector::CxxHeaderPathDetector(const std::string& compilerName)
	: PathDetector(compilerName), m_compilerName(compilerName)
{
}

std::vector<FilePath> CxxHeaderPathDetector::doGetPaths() const
{
	std::vector<std::string> paths = utility::getCxxHeaderPaths(m_compilerName);
	std::vector<FilePath> headerSearchPaths;

	for (const std::string& path: paths)
	{
		if (!utility::isPostfix(" (framework directory)", path) &&
			FilePath(path).getCanonical().exists() &&
			!FilePath(path).getCanonical().getConcatenated("/stdarg.h").exists())
		{
			headerSearchPaths.push_back(FilePath(path).makeCanonical());
		}
	}

	return headerSearchPaths;
}
