#include "SourceGroupSettingsWithSourceExtensions.h"

#include "ProjectSettings.h"
#include "utility.h"

std::vector<std::string> SourceGroupSettingsWithSourceExtensions::getSourceExtensions() const
{
	if (m_sourceExtensions.empty())
	{
		return getDefaultSourceExtensions();
	}
	return m_sourceExtensions;
}

void SourceGroupSettingsWithSourceExtensions::setSourceExtensions(
	const std::vector<std::string>& sourceExtensions)
{
	m_sourceExtensions = sourceExtensions;
}

bool SourceGroupSettingsWithSourceExtensions::equals(const SourceGroupSettingsBase* other) const
{
	const SourceGroupSettingsWithSourceExtensions* otherPtr =
		dynamic_cast<const SourceGroupSettingsWithSourceExtensions*>(other);

	return (otherPtr && utility::isPermutation(m_sourceExtensions, otherPtr->m_sourceExtensions));
}

void SourceGroupSettingsWithSourceExtensions::load(const ConfigManager* config, const std::string& key)
{
	setSourceExtensions(config->getValuesOrDefaults(
		key + "/source_extensions/source_extension", std::vector<std::string>()));
}

void SourceGroupSettingsWithSourceExtensions::save(ConfigManager* config, const std::string& key)
{
	config->setValues(key + "/source_extensions/source_extension", getSourceExtensions());
}
