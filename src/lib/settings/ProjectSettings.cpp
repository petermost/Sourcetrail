#include "ProjectSettings.h"

#include "SourceGroupSettingsCustomCommand.h"
#include "SourceGroupSettingsUnloadable.h"
#include "logging.h"
#include "utilityFile.h"
#include "utilityString.h"
#include "utilityUuid.h"

#if BUILD_CXX_LANGUAGE_PACKAGE
#	include "SourceGroupSettingsCEmpty.h"
#	include "SourceGroupSettingsCppEmpty.h"
#	include "SourceGroupSettingsCxxCdb.h"
#	include "SourceGroupSettingsCxxCodeblocks.h"
#endif	  // BUILD_CXX_LANGUAGE_PACKAGE

#if BUILD_JAVA_LANGUAGE_PACKAGE
#	include "SourceGroupSettingsJavaEmpty.h"
#	include "SourceGroupSettingsJavaGradle.h"
#	include "SourceGroupSettingsJavaMaven.h"
#endif	  // BUILD_JAVA_LANGUAGE_PACKAGE

const std::string ProjectSettings::PROJECT_FILE_EXTENSION = ".srctrlprj";
const std::string ProjectSettings::BOOKMARK_DB_FILE_EXTENSION = ".srctrlbm";
const std::string ProjectSettings::INDEX_DB_FILE_EXTENSION = ".srctrldb";
const std::string ProjectSettings::TEMP_INDEX_DB_FILE_EXTENSION = ".srctrldb_tmp";

const size_t ProjectSettings::VERSION = 8;

LanguageType ProjectSettings::getLanguageOfProject(const FilePath& filePath)
{
	LanguageType languageType = LanguageType::UNKNOWN;

	ProjectSettings projectSettings;
	projectSettings.load(filePath);
	for (const std::shared_ptr<SourceGroupSettings>& sourceGroupSettings:
		 projectSettings.getAllSourceGroupSettings())
	{
		const LanguageType currentLanguageType = getLanguageTypeForSourceGroupType(
			sourceGroupSettings->getType());
		if (languageType == LanguageType::UNKNOWN)
		{
			languageType = currentLanguageType;
		}
		else if (languageType != currentLanguageType)
		{
			// language is unknown if source groups have different languages.
			languageType = LanguageType::UNKNOWN;
			break;
		}
	}

	return languageType;
}

ProjectSettings::ProjectSettings() = default;

ProjectSettings::ProjectSettings(const FilePath& projectFilePath)
{
	setFilePath(projectFilePath);
}

ProjectSettings::~ProjectSettings() = default;

bool ProjectSettings::equalsExceptNameAndLocation(const ProjectSettings& other) const
{
	const std::vector<std::shared_ptr<SourceGroupSettings>> allMySettings =
		getAllSourceGroupSettings();
	const std::vector<std::shared_ptr<SourceGroupSettings>> allOtherSettings =
		other.getAllSourceGroupSettings();

	if (allMySettings.size() != allOtherSettings.size())
	{
		return false;
	}

	for (const std::shared_ptr<SourceGroupSettings>& mySourceGroup: allMySettings)
	{
		bool matched = false;
		for (const std::shared_ptr<SourceGroupSettings>& otherSourceGroup: allOtherSettings)
		{
			if (mySourceGroup->equalsSettings(otherSourceGroup.get()))
			{
				matched = true;
				break;
			}
		}

		if (!matched)
		{
			return false;
		}
	}

	return true;
}

bool ProjectSettings::reload()
{
	return Settings::load(getFilePath());
}

FilePath ProjectSettings::getProjectFilePath() const
{
	return getFilePath();
}

void ProjectSettings::setProjectFilePath(std::string projectName, const FilePath& projectFileLocation)
{
	setFilePath(projectFileLocation.getConcatenated("/" + projectName + PROJECT_FILE_EXTENSION));
}

FilePath ProjectSettings::getDependenciesDirectoryPath() const
{
	return getProjectDirectoryPath().concatenate("sourcetrail_dependencies");
}

FilePath ProjectSettings::getDBFilePath() const
{
	return getFilePath().replaceExtension(INDEX_DB_FILE_EXTENSION);
}

FilePath ProjectSettings::getTempDBFilePath() const
{
	return getFilePath().replaceExtension(TEMP_INDEX_DB_FILE_EXTENSION);
}

FilePath ProjectSettings::getBookmarkDBFilePath() const
{
	return getFilePath().replaceExtension(BOOKMARK_DB_FILE_EXTENSION);
}

std::string ProjectSettings::getProjectName() const
{
	return getFilePath().withoutExtension().fileName();
}

FilePath ProjectSettings::getProjectDirectoryPath() const
{
	return getFilePath().getParentDirectory();
}

std::string ProjectSettings::getDescription() const
{
	return getValue<std::string>("description", "");
}

std::vector<std::shared_ptr<SourceGroupSettings>> ProjectSettings::getAllSourceGroupSettings() const
{
	std::vector<std::shared_ptr<SourceGroupSettings>> allSettings;
	for (const std::string& key: m_config->getSublevelKeys("source_groups"))
	{
		const std::string id = key.substr(std::string(SourceGroupSettings::s_keyPrefix).length());
		const SourceGroupType type = stringToSourceGroupType(
			getValue<std::string>(key + "/type", ""));

		std::shared_ptr<SourceGroupSettings> settings;

		switch (type)
		{
#if BUILD_CXX_LANGUAGE_PACKAGE
		case SourceGroupType::C_EMPTY:
			settings = std::make_shared<SourceGroupSettingsCEmpty>(id, this);
			break;
		case SourceGroupType::CXX_EMPTY:
			settings = std::make_shared<SourceGroupSettingsCppEmpty>(id, this);
			break;
		case SourceGroupType::CXX_CDB:
			settings = std::make_shared<SourceGroupSettingsCxxCdb>(id, this);
			break;
		case SourceGroupType::CXX_CODEBLOCKS:
			settings = std::make_shared<SourceGroupSettingsCxxCodeblocks>(id, this);
			break;
#endif	  // BUILD_CXX_LANGUAGE_PACKAGE
#if BUILD_JAVA_LANGUAGE_PACKAGE
		case SourceGroupType::JAVA_EMPTY:
			settings = std::make_shared<SourceGroupSettingsJavaEmpty>(id, this);
			break;
		case SourceGroupType::JAVA_MAVEN:
			settings = std::make_shared<SourceGroupSettingsJavaMaven>(id, this);
			break;
		case SourceGroupType::JAVA_GRADLE:
			settings = std::make_shared<SourceGroupSettingsJavaGradle>(id, this);
			break;
#endif	  // BUILD_JAVA_LANGUAGE_PACKAGE
		case SourceGroupType::CUSTOM_COMMAND:
			settings = std::make_shared<SourceGroupSettingsCustomCommand>(id, this);
			break;
		default:
			settings = std::make_shared<SourceGroupSettingsUnloadable>(id, this);
		}

		if (settings)
		{
			settings->loadSettings(m_config.get());
			allSettings.push_back(settings);
		}
		else
		{
			LOG_WARNING("Sourcegroup with id \"" + id + "\" could not be loaded.");
		}
	}

	return allSettings;
}

void ProjectSettings::setAllSourceGroupSettings(
	const std::vector<std::shared_ptr<SourceGroupSettings>>& allSettings)
{
	for (const std::string& key: m_config->getSublevelKeys("source_groups"))
	{
		m_config->removeValues(key);
	}

	for (const std::shared_ptr<SourceGroupSettings>& settings: allSettings)
	{
		const std::string key = SourceGroupSettings::s_keyPrefix + settings->getId();
		const SourceGroupType type = settings->getType();
		setValue(key + "/type", sourceGroupTypeToString(type));

		settings->saveSettings(m_config.get());
	}
}

std::vector<FilePath> ProjectSettings::makePathsExpandedAndAbsolute(const std::vector<FilePath>& paths) const
{
	std::vector<FilePath> p = utility::getExpandedPaths(paths);

	std::vector<FilePath> absPaths;
	const FilePath basePath = getProjectDirectoryPath();
	for (const FilePath& path: p)
	{
		if (path.isAbsolute())
		{
			absPaths.push_back(path);
		}
		else
		{
			absPaths.push_back(basePath.getConcatenated(path).makeCanonical());
		}
	}

	return absPaths;
}

FilePath ProjectSettings::makePathExpandedAndAbsolute(const FilePath& path) const
{
	return utility::getExpandedAndAbsolutePath(path, getProjectDirectoryPath());
}
