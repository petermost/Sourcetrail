#ifndef SOURCE_GROUP_SETTINGS_WITH_EXCLUDE_FILTERS_H
#define SOURCE_GROUP_SETTINGS_WITH_EXCLUDE_FILTERS_H

#include <vector>

#include "SourceGroupSettingsComponent.h"

class FilePathFilter;

class SourceGroupSettingsWithExcludeFilters: public SourceGroupSettingsComponent
{
public:
	~SourceGroupSettingsWithExcludeFilters() override = default;

	std::vector<std::string> getExcludeFilterStrings() const;
	std::vector<FilePathFilter> getExcludeFiltersExpandedAndAbsolute() const;
	void setExcludeFilterStrings(const std::vector<std::string>& excludeFilters);

protected:
	bool equals(const SourceGroupSettingsBase* other) const override;

	void load(const ConfigManager* config, const std::string& key) override;
	void save(ConfigManager* config, const std::string& key) override;

private:
	std::vector<FilePathFilter> getFiltersExpandedAndAbsolute(
		const std::vector<std::string>& filterStrings) const;

	std::vector<std::string> m_excludeFilters;
};

#endif	  // SOURCE_GROUP_SETTINGS_WITH_EXCLUDE_FILTERS_H
