#include "SourceGroupSettingsWithCppStandard.h"

#include "ConfigManager.h"

#include <ToolVersionSupport.h>

std::string SourceGroupSettingsWithCppStandard::getDefaultCppStandard()
{
	return ClangVersionSupport::getLatestCppStandard();
}

std::vector<std::string> SourceGroupSettingsWithCppStandard::getAvailableCppStandards()
{
	return ClangVersionSupport::getAvailableCppStandards();
}

std::string SourceGroupSettingsWithCppStandard::getCppStandard() const
{
	if (m_cppStandard.empty())
	{
		return getDefaultCppStandard();
	}
	return m_cppStandard;
}

void SourceGroupSettingsWithCppStandard::setCppStandard(const std::string& standard)
{
	m_cppStandard = standard;
}

bool SourceGroupSettingsWithCppStandard::equals(const SourceGroupSettingsBase* other) const
{
	const SourceGroupSettingsWithCppStandard* otherPtr =
		dynamic_cast<const SourceGroupSettingsWithCppStandard*>(other);

	return (otherPtr && m_cppStandard == otherPtr->m_cppStandard);
}

void SourceGroupSettingsWithCppStandard::load(const ConfigManager* config, const std::string& key)
{
	setCppStandard(config->getValueOrDefault<std::string>(key + "/cpp_standard", ""));
}

void SourceGroupSettingsWithCppStandard::save(ConfigManager* config, const std::string& key)
{
	config->setValue(key + "/cpp_standard", getCppStandard());
}
