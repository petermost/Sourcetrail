#ifndef PROJECT_SETTINGS_H
#define PROJECT_SETTINGS_H

#include <memory>
#include <vector>

#include "LanguageType.h"
#include "Settings.h"
#include "SettingsMigrator.h"

class SourceGroupSettings;

class ProjectSettings: public Settings
{
public:
	static const std::string PROJECT_FILE_EXTENSION;
	static const std::string BOOKMARK_DB_FILE_EXTENSION;
	static const std::string INDEX_DB_FILE_EXTENSION;
	static const std::string TEMP_INDEX_DB_FILE_EXTENSION;

	static const size_t VERSION;
	static LanguageType getLanguageOfProject(const FilePath& filePath);

	ProjectSettings();
	ProjectSettings(const FilePath& projectFilePath);
	~ProjectSettings() override;

	bool equalsExceptNameAndLocation(const ProjectSettings& other) const;

	bool needMigration() const;
	void migrate();

	bool reload();

	FilePath getProjectFilePath() const;
	void setProjectFilePath(std::string projectName, const FilePath& projectFileLocation);
	FilePath getDependenciesDirectoryPath() const;

	FilePath getDBFilePath() const;
	FilePath getTempDBFilePath() const;
	FilePath getBookmarkDBFilePath() const;

	std::string getProjectName() const;
	FilePath getProjectDirectoryPath() const;

	std::string getDescription() const;

	std::vector<std::shared_ptr<SourceGroupSettings>> getAllSourceGroupSettings() const;
	void setAllSourceGroupSettings(const std::vector<std::shared_ptr<SourceGroupSettings>>& allSettings);

	std::vector<FilePath> makePathsExpandedAndAbsolute(const std::vector<FilePath>& paths) const;
	FilePath makePathExpandedAndAbsolute(const FilePath& path) const;

private:
	SettingsMigrator getMigrations() const;
};

#endif	  // PROJECT_SETTINGS_H
