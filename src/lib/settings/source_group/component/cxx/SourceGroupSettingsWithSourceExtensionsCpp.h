#ifndef SOURCE_GROUP_SETTINGS_WITH_SOURCE_EXTENSIONS_CPP_H
#define SOURCE_GROUP_SETTINGS_WITH_SOURCE_EXTENSIONS_CPP_H

#include "SourceGroupSettingsWithSourceExtensions.h"

class SourceGroupSettingsWithSourceExtensionsCpp: public SourceGroupSettingsWithSourceExtensions
{
private:
	std::vector<std::string> getDefaultSourceExtensions() const override
	{
		return {".cpp", ".cxx", ".cc"};
	}
};

#endif	  // SOURCE_GROUP_SETTINGS_WITH_SOURCE_EXTENSIONS_CPP_H
