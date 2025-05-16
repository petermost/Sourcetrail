#ifndef SOURCE_GROUP_SETTINGS_WITH_SOURCE_EXTENSIONS_H
#define SOURCE_GROUP_SETTINGS_WITH_SOURCE_EXTENSIONS_H

#include <vector>

#include "SourceGroupSettingsComponent.h"

class SourceGroupSettingsWithSourceExtensions: public SourceGroupSettingsComponent
{
public:
	~SourceGroupSettingsWithSourceExtensions() override = default;

	std::vector<std::string> getSourceExtensions() const;
	void setSourceExtensions(const std::vector<std::string>& sourceExtensions);

protected:
	bool equals(const SourceGroupSettingsBase* other) const override;

	void load(const ConfigManager* config, const std::string& key) override;
	void save(ConfigManager* config, const std::string& key) override;

private:
	virtual std::vector<std::string> getDefaultSourceExtensions() const = 0;

	std::vector<std::string> m_sourceExtensions;
};

#endif	  // SOURCE_GROUP_SETTINGS_WITH_SOURCE_EXTENSIONS_H
