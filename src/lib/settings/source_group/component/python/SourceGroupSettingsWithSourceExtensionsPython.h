#ifndef SOURCE_GROUP_SETTINGS_WITH_SOURCE_EXTENSIONS_PYTHON_H
#define SOURCE_GROUP_SETTINGS_WITH_SOURCE_EXTENSIONS_PYTHON_H

#include "SourceGroupSettingsWithSourceExtensions.h"

class SourceGroupSettingsWithSourceExtensionsPython: public SourceGroupSettingsWithSourceExtensions
{
private:
	std::vector<std::string> getDefaultSourceExtensions() const override
	{
		return {".py"};
	}
};

#endif	  // SOURCE_GROUP_SETTINGS_WITH_SOURCE_EXTENSIONS_PYTHON_H
