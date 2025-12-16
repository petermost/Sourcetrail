#include "SourceGroupSettingsWithCStandard.h"

#include "ConfigManager.h"
#include "ToolChain.h"

std::string SourceGroupSettingsWithCStandard::getDefaultCStandard()
{
	return ClangCompiler::getLatestCStandard();
}

std::vector<std::string> SourceGroupSettingsWithCStandard::getAvailableCStandards()
{
	return ClangCompiler::getAvailableCStandards();
}

std::string SourceGroupSettingsWithCStandard::getCStandard() const
{
	if (m_cStandard.empty())
	{
		return getDefaultCStandard();
	}
	return m_cStandard;
}

void SourceGroupSettingsWithCStandard::setCStandard(const std::string& standard)
{
	m_cStandard = standard;
}


bool SourceGroupSettingsWithCStandard::equals(const SourceGroupSettingsBase* other) const
{
	const SourceGroupSettingsWithCStandard* otherPtr =
		dynamic_cast<const SourceGroupSettingsWithCStandard*>(other);

	return (otherPtr && m_cStandard == otherPtr->m_cStandard);
}

void SourceGroupSettingsWithCStandard::load(const ConfigManager* config, const std::string& key)
{
	setCStandard(config->getValueOrDefault<std::string>(key + "/c_standard", ""));
}

void SourceGroupSettingsWithCStandard::save(ConfigManager* config, const std::string& key)
{
	config->setValue(key + "/c_standard", getCStandard());
}
