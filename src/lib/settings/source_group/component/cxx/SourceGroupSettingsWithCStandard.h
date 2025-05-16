#ifndef SOURCE_GROUP_SETTINGS_WITH_C_STANDARD_H
#define SOURCE_GROUP_SETTINGS_WITH_C_STANDARD_H

#include <vector>

#include "SourceGroupSettingsComponent.h"

class SourceGroupSettingsWithCStandard: public SourceGroupSettingsComponent
{
public:
	static std::string getDefaultCStandard();
	static std::vector<std::string> getAvailableCStandards();

	~SourceGroupSettingsWithCStandard() override = default;

	std::string getCStandard() const;
	void setCStandard(const std::string& standard);

protected:
	bool equals(const SourceGroupSettingsBase* other) const override;

	void load(const ConfigManager* config, const std::string& key) override;
	void save(ConfigManager* config, const std::string& key) override;

private:
	std::string m_cStandard;
};

#endif	  // SOURCE_GROUP_SETTINGS_WITH_C_STANDARD_H
