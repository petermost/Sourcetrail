#include "FileManager.h"

#include <set>

#include "FilePath.h"
#include "FilePathFilter.h"
#include "FileSystem.h"

FileManager::FileManager() = default;

FileManager::~FileManager() = default;

void FileManager::update(
	const std::vector<FilePath>& sourcePaths,
	const std::vector<FilePathFilter>& excludeFilters,
	const std::vector<std::string>& sourceExtensions)
{
	m_sourcePaths = sourcePaths;
	m_excludeFilters = excludeFilters;
	m_sourceExtensions = sourceExtensions;

	m_allSourceFilePaths.clear();

	for (const FileInfo& fileInfo:
		 FileSystem::getFileInfosFromPaths(m_sourcePaths, m_sourceExtensions))
	{
		const FilePath& filePath = fileInfo.path;
		if (isExcluded(filePath))
		{
			continue;
		}

		m_allSourceFilePaths.insert(filePath);
	}
}

std::vector<FilePath> FileManager::getSourcePaths() const
{
	return m_sourcePaths;
}

bool FileManager::hasSourceFilePath(const FilePath& filePath) const
{
	return m_allSourceFilePaths.find(filePath) != m_allSourceFilePaths.end();
}

std::set<FilePath> FileManager::getAllSourceFilePaths() const
{
	return m_allSourceFilePaths;
}

bool FileManager::isExcluded(const FilePath& filePath) const
{
	return FilePathFilter::areMatching(m_excludeFilters, filePath);
}
