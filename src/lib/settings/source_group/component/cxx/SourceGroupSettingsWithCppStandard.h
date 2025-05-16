#ifndef SOURCE_GROUP_SETTINGS_WITH_CPP_STANDARD_H
#define SOURCE_GROUP_SETTINGS_WITH_CPP_STANDARD_H

#include <vector>

#include "SourceGroupSettingsComponent.h"

class SourceGroupSettingsWithCppStandard: public SourceGroupSettingsComponent
{
public:
	static std::string getDefaultCppStandard();
	static std::vector<std::string> getAvailableCppStandards();

	~SourceGroupSettingsWithCppStandard() override = default;

	std::string getCppStandard() const;
	void setCppStandard(const std::string& standard);

protected:
	bool equals(const SourceGroupSettingsBase* other) const override;

	void load(const ConfigManager* config, const std::string& key) override;
	void save(ConfigManager* config, const std::string& key) override;

private:
	std::string m_cppStandard;
};

#endif	  // SOURCE_GROUP_SETTINGS_WITH_CPP_STANDARD_H
