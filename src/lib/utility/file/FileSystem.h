#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <set>
#include <string>
#include <vector>

#include "FileInfo.h"
#include "TimeStamp.h"

class FileSystem
{
public:
	static std::vector<FilePath> getFilePathsFromDirectory(
		const FilePath& path, const std::vector<std::string>& extensions = {});

	static FileInfo getFileInfoForPath(const FilePath& filePath);

	static std::vector<FileInfo> getFileInfosFromPaths(
		const std::vector<FilePath>& paths,
		const std::vector<std::string>& fileExtensions,
		bool followSymLinks = true);

	static std::set<FilePath> getSymLinkedDirectories(const FilePath& path);
	static std::set<FilePath> getSymLinkedDirectories(const std::vector<FilePath>& paths);

	static unsigned long long getFileByteSize(const FilePath& filePath);

	static TimeStamp getLastWriteTime(const FilePath& filePath);

	static bool remove(const FilePath& path);
	static bool rename(const FilePath& from, const FilePath& to);

	/** This method does nothing if the copy destination already exist. */
	static bool copyFile(const FilePath& from, const FilePath& to);

	/** This method does nothing if the copy destination already exist. */
	static bool copyDirectory(const FilePath &from, const FilePath &to);

	static void createDirectories(const FilePath& path);
	static std::vector<FilePath> getDirectSubDirectories(const FilePath& path);
};

#endif	  // FILE_SYSTEM_H
