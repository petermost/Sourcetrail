#include "AppPath.h"

#include "utilityApp.h"

FilePath AppPath::s_sharedDataDirectoryPath("");
FilePath AppPath::s_cxxIndexerDirectoryPath("");

FilePath AppPath::getSharedDataDirectoryPath()
{
	return s_sharedDataDirectoryPath;
}

void AppPath::setSharedDataDirectoryPath(const FilePath& path)
{
	s_sharedDataDirectoryPath = path;
}

FilePath AppPath::getCxxIndexerFilePath()
{
	std::string cxxIndexerName("sourcetrail_indexer" + FilePath::getExecutableExtension());

	if (!s_cxxIndexerDirectoryPath.empty())
		return s_cxxIndexerDirectoryPath.getConcatenated(cxxIndexerName);
	else
		return s_sharedDataDirectoryPath.getConcatenated(cxxIndexerName);
}

void AppPath::setCxxIndexerDirectoryPath(const FilePath& path)
{
	s_cxxIndexerDirectoryPath = path;
}
