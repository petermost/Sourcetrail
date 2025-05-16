#include "ResourcePaths.h"

#include "AppPath.h"

FilePath ResourcePaths::getColorSchemesDirectoryPath()
{
	return AppPath::getSharedDataDirectoryPath().concatenate("data/color_schemes/");
}

FilePath ResourcePaths::getSyntaxHighlightingRulesDirectoryPath()
{
	return AppPath::getSharedDataDirectoryPath().concatenate("data/syntax_highlighting_rules/");
}

FilePath ResourcePaths::getFallbackDirectoryPath()
{
	return AppPath::getSharedDataDirectoryPath().concatenate("data/fallback/");
}

FilePath ResourcePaths::getFontsDirectoryPath()
{
	return AppPath::getSharedDataDirectoryPath().concatenate("data/fonts/");
}

FilePath ResourcePaths::getJavaDirectoryPath()
{
	return AppPath::getSharedDataDirectoryPath().concatenate("data/java/");
}

FilePath ResourcePaths::getPythonDirectoryPath()
{
	return AppPath::getSharedDataDirectoryPath().concatenate("data/python/");
}

FilePath ResourcePaths::getCxxCompilerHeaderDirectoryPath()
{
	return AppPath::getSharedDataDirectoryPath().concatenate("data/cxx/include/").getCanonical();
}

FilePath ResourcePaths::getPythonIndexerFilePath()
{
	return getPythonDirectoryPath().concatenate("SourcetrailPythonIndexer" + FilePath::getExecutableExtension());
}
