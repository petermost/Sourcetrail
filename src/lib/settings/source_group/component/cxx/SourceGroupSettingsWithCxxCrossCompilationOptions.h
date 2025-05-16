#ifndef SOURCE_GROUP_SETTINGS_WITH_CXX_CROSS_COMPILATION_OPTIONS_H
#define SOURCE_GROUP_SETTINGS_WITH_CXX_CROSS_COMPILATION_OPTIONS_H

#include <vector>

#include "SourceGroupSettingsComponent.h"

class SourceGroupSettingsWithCxxCrossCompilationOptions: public SourceGroupSettingsComponent
{
public:
	static std::vector<std::string> getAvailableArchTypes();
	static std::vector<std::string> getAvailableVendorTypes();
	static std::vector<std::string> getAvailableOsTypes();
	static std::vector<std::string> getAvailableEnvironmentTypes();

	~SourceGroupSettingsWithCxxCrossCompilationOptions() override = default;

	bool getTargetOptionsEnabled() const;
	void setTargetOptionsEnabled(bool targetOptionsEnabled);

	std::string getTargetArch() const;
	void setTargetArch(const std::string& arch);

	std::string getTargetVendor() const;
	void setTargetVendor(const std::string& vendor);

	std::string getTargetSys() const;
	void setTargetSys(const std::string& sys);

	std::string getTargetAbi() const;
	void setTargetAbi(const std::string& abi);

	std::string getTargetFlag() const;

protected:
	bool equals(const SourceGroupSettingsBase* other) const override;

	void load(const ConfigManager* config, const std::string& key) override;
	void save(ConfigManager* config, const std::string& key) override;

private:
	bool m_targetOptionsEnabled = false;
	std::string m_targetArch;
	std::string m_targetVendor;
	std::string m_targetSys;
	std::string m_targetAbi;
};

#endif	  // SOURCE_GROUP_SETTINGS_WITH_CXX_CROSS_COMPILATION_OPTIONS_H
