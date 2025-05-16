#ifndef SOURCE_GROUP_SETTINGS_WITH_SOURCE_EXTENSIONS_JAVA_H
#define SOURCE_GROUP_SETTINGS_WITH_SOURCE_EXTENSIONS_JAVA_H

#include "SourceGroupSettingsWithSourceExtensions.h"

class SourceGroupSettingsWithSourceExtensionsJava: public SourceGroupSettingsWithSourceExtensions
{
private:
	std::vector<std::string> getDefaultSourceExtensions() const override
	{
		return {".java"};
	}
};

#endif	  // SOURCE_GROUP_SETTINGS_WITH_SOURCE_EXTENSIONS_JAVA_H
