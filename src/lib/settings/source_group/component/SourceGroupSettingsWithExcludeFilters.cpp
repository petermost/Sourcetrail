#include "SourceGroupSettingsWithExcludeFilters.h"

#include "FilePathFilter.h"
#include "FileSystem.h"
#include "ProjectSettings.h"
#include "utility.h"
#include "utilityFile.h"

std::vector<std::string> SourceGroupSettingsWithExcludeFilters::getExcludeFilterStrings() const
{
	return m_excludeFilters;
}

std::vector<FilePathFilter> SourceGroupSettingsWithExcludeFilters::getExcludeFiltersExpandedAndAbsolute() const
{
	return getFiltersExpandedAndAbsolute(getExcludeFilterStrings());
}

void SourceGroupSettingsWithExcludeFilters::setExcludeFilterStrings(
	const std::vector<std::string>& excludeFilters)
{
	m_excludeFilters = excludeFilters;
}

bool SourceGroupSettingsWithExcludeFilters::equals(const SourceGroupSettingsBase* other) const
{
	const SourceGroupSettingsWithExcludeFilters* otherPtr =
		dynamic_cast<const SourceGroupSettingsWithExcludeFilters*>(other);

	return (otherPtr && utility::isPermutation(m_excludeFilters, otherPtr->m_excludeFilters));
}

void SourceGroupSettingsWithExcludeFilters::load(const ConfigManager* config, const std::string& key)
{
	setExcludeFilterStrings(config->getValuesOrDefaults(
		key + "/exclude_filters/exclude_filter", std::vector<std::string>()));
}

void SourceGroupSettingsWithExcludeFilters::save(ConfigManager* config, const std::string& key)
{
	config->setValues(key + "/exclude_filters/exclude_filter", getExcludeFilterStrings());
}

std::vector<FilePathFilter> SourceGroupSettingsWithExcludeFilters::getFiltersExpandedAndAbsolute(
	const std::vector<std::string>& filterStrings) const
{
	const FilePath projectDirectoryPath = getProjectSettings()->getProjectDirectoryPath();

	std::vector<FilePathFilter> result;

	for (const std::string& filterString: filterStrings)
	{
		if (!filterString.empty())
		{
			const size_t wildcardPos = filterString.find("*");
			if (wildcardPos != std::string::npos)
			{
				std::smatch match;
				if (std::regex_search(filterString, match, std::regex("[\\\\/]")) &&
					!match.empty() && match.position(0) < int(wildcardPos))
				{
					const FilePath p = utility::getExpandedAndAbsolutePath(
						FilePath(match.prefix().str()), projectDirectoryPath);
					std::set<FilePath> symLinkPaths = FileSystem::getSymLinkedDirectories(p);
					symLinkPaths.insert(p);

					utility::append(
						result,
						utility::convert<FilePath, FilePathFilter>(
							utility::toVector(symLinkPaths), [match](const FilePath& filePath) {
								return FilePathFilter(filePath.str() + "/" + match.suffix().str());
							}));
				}
				else
				{
					result.push_back(FilePathFilter(filterString));
				}
			}
			else
			{
				const FilePath p = utility::getExpandedAndAbsolutePath(
					FilePath(filterString), projectDirectoryPath);
				const bool isFile = p.exists() && !p.isDirectory();

				std::set<FilePath> symLinkPaths = FileSystem::getSymLinkedDirectories(p);
				symLinkPaths.insert(p);

				utility::append(
					result,
					utility::convert<FilePath, FilePathFilter>(
						utility::toVector(symLinkPaths), [isFile](const FilePath& filePath) {
							return FilePathFilter(filePath.str() + (isFile ? "" : "**"));
						}));
			}
		}
	}

	return result;
}
