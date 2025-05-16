#ifndef SOURCE_GROUP_SETTINGS_WITH_CXX_PCH_OPTIONS_H
#define SOURCE_GROUP_SETTINGS_WITH_CXX_PCH_OPTIONS_H

#include "FilePath.h"
#include "SourceGroupSettingsComponent.h"

class SourceGroupSettingsWithCxxPchOptions: public SourceGroupSettingsComponent
{
public:
	~SourceGroupSettingsWithCxxPchOptions() override = default;

	FilePath getPchDependenciesDirectoryPath() const;

	FilePath getPchInputFilePath() const;
	FilePath getPchInputFilePathExpandedAndAbsolute() const;
	void setPchInputFilePathFilePath(const FilePath& path);

	std::vector<std::string> getPchFlags() const;
	void setPchFlags(const std::vector<std::string>& pchFlags);

	bool getUseCompilerFlags() const;
	void setUseCompilerFlags(bool useCompilerFlags);

protected:
	bool equals(const SourceGroupSettingsBase* other) const override;

	void load(const ConfigManager* config, const std::string& key) override;
	void save(ConfigManager* config, const std::string& key) override;

private:
	FilePath m_pchInputFilePath;
	std::vector<std::string> m_pchFlags;
	bool m_useCompilerFlags = true;
};

#endif	  // SOURCE_GROUP_SETTINGS_WITH_CXX_PCH_OPTIONS_H
