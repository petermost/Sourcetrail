#ifndef SOURCE_GROUP_SETTINGS_WITH_JAVA_STANDARD_H
#define SOURCE_GROUP_SETTINGS_WITH_JAVA_STANDARD_H

#include "SourceGroupSettingsComponent.h"

class SourceGroupSettingsWithJavaStandard: public SourceGroupSettingsComponent
{
public:
	~SourceGroupSettingsWithJavaStandard() override = default;

	std::string getJavaStandard() const;
	void setJavaStandard(const std::string& standard);

protected:
	bool equals(const SourceGroupSettingsBase* other) const override;

	void load(const ConfigManager* config, const std::string& key) override;
	void save(ConfigManager* config, const std::string& key) override;

private:
	std::string m_javaStandard;
};

#endif	  // SOURCE_GROUP_SETTINGS_WITH_JAVA_STANDARD_H
