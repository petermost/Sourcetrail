#ifndef RESOURCE_PATHS_H
#define RESOURCE_PATHS_H

#include "FilePath.h"

class ResourcePaths
{
public:
	static FilePath getColorSchemesDirectoryPath();
	static FilePath getSyntaxHighlightingRulesDirectoryPath();
	static FilePath getFallbackDirectoryPath();
	static FilePath getFontsDirectoryPath();
	static FilePath getJavaDirectoryPath();
	static FilePath getCxxCompilerHeaderDirectoryPath();
};

#endif	  // RESOURCE_PATHS_H
