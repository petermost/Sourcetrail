#include "FileSystem.h"

#include <set>

#include <boost/date_time.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>
#include <boost/filesystem.hpp>

#include "utilityString.h"

std::vector<FilePath> FileSystem::getFilePathsFromDirectory(
	const FilePath& path, const std::vector<std::string>& extensions)
{
	std::set<std::string> ext(extensions.begin(), extensions.end());
	std::vector<FilePath> files;

	if (path.isDirectory())
	{
		boost::filesystem::recursive_directory_iterator it(path.getPath());
		boost::filesystem::recursive_directory_iterator endit;
		while (it != endit)
		{
			if (boost::filesystem::is_symlink(*it))
			{
				// check for self-referencing symlinks
				boost::filesystem::path p = boost::filesystem::read_symlink(*it);
				if (p.filename() == p.string() && p.filename() == it->path().filename())
				{
					++it;
					continue;
				}
			}

			if (boost::filesystem::is_regular_file(*it) &&
				(ext.empty() || ext.find(it->path().extension().string()) != ext.end()))
			{
				files.push_back(FilePath(it->path().generic_string()));
			}
			++it;
		}
	}
	return files;
}

FileInfo FileSystem::getFileInfoForPath(const FilePath& filePath)
{
	if (filePath.exists())
	{
		return FileInfo(filePath, getLastWriteTime(filePath));
	}
	return FileInfo();
}

std::vector<FileInfo> FileSystem::getFileInfosFromPaths(
	const std::vector<FilePath>& paths,
	const std::vector<std::string>& fileExtensions,
	bool followSymLinks)
{
	std::set<std::string> ext;
	for (const std::string& e: fileExtensions)
	{
		ext.insert(utility::toLowerCase(e));
	}

	std::set<boost::filesystem::path> symlinkDirs;
	std::set<FilePath> filePaths;

	std::vector<FileInfo> files;

	for (const FilePath& path: paths)
	{
		if (path.isDirectory())
		{
			boost::filesystem::recursive_directory_iterator it(
				path.getPath(), boost::filesystem::directory_options::follow_directory_symlink);
			boost::filesystem::recursive_directory_iterator endit;
			boost::system::error_code ec;
			for (; it != endit; it.increment(ec))
			{
				if (boost::filesystem::is_symlink(*it))
				{
					if (!followSymLinks)
					{
						it.disable_recursion_pending();
						continue;
					}

					// check for self-referencing symlinks
					boost::filesystem::path p = boost::filesystem::read_symlink(*it);
					if (p.filename() == p.string() && p.filename() == it->path().filename())
					{
						continue;
					}

					// check for duplicates when following directory symlinks
					if (boost::filesystem::is_directory(*it))
					{
						boost::filesystem::path absDir = boost::filesystem::canonical(
							p, it->path().parent_path());

						if (symlinkDirs.find(absDir) != symlinkDirs.end())
						{
							it.disable_recursion_pending();
							continue;
						}

						symlinkDirs.insert(absDir);
					}
				}

				if (boost::filesystem::is_regular_file(*it) &&
					(ext.empty() ||
					 ext.find(utility::toLowerCase(it->path().extension().string())) != ext.end()))
				{
					const FilePath canonicalPath = FilePath(it->path().string()).getCanonical();
					if (filePaths.find(canonicalPath) != filePaths.end())
					{
						continue;
					}
					filePaths.insert(canonicalPath);
					files.push_back(getFileInfoForPath(canonicalPath));
				}
			}
		}
		else if (path.exists() && (ext.empty() || ext.find(utility::toLowerCase(path.extension())) != ext.end()))
		{
			const FilePath canonicalPath = path.getCanonical();
			if (filePaths.find(canonicalPath) != filePaths.end())
			{
				continue;
			}
			filePaths.insert(canonicalPath);
			files.push_back(getFileInfoForPath(canonicalPath));
		}
	}

	return files;
}

std::set<FilePath> FileSystem::getSymLinkedDirectories(const FilePath& path)
{
	return getSymLinkedDirectories(std::vector<FilePath> {path});
}

std::set<FilePath> FileSystem::getSymLinkedDirectories(const std::vector<FilePath>& paths)
{
	std::set<boost::filesystem::path> symlinkDirs;

	for (const FilePath& path: paths)
	{
		if (path.isDirectory())
		{
			boost::filesystem::recursive_directory_iterator it(
				path.getPath(), boost::filesystem::directory_options::follow_directory_symlink);
			boost::filesystem::recursive_directory_iterator endit;
			boost::system::error_code ec;
			for (; it != endit; it.increment(ec))
			{
				if (boost::filesystem::is_symlink(*it))
				{
					// check for self-referencing symlinks
					boost::filesystem::path p = boost::filesystem::read_symlink(*it);
					if (p.filename() == p.string() && p.filename() == it->path().filename())
					{
						continue;
					}

					// check for duplicates when following directory symlinks
					if (boost::filesystem::is_directory(*it))
					{
						boost::filesystem::path absDir = boost::filesystem::canonical(
							p, it->path().parent_path());

						if (symlinkDirs.find(absDir) != symlinkDirs.end())
						{
							it.disable_recursion_pending();
							continue;
						}

						symlinkDirs.insert(absDir);
					}
				}
			}
		}
	}

	std::set<FilePath> files;
	for (const auto& p: symlinkDirs)
	{
		files.insert(FilePath(p.string()));
	}
	return files;
}

unsigned long long FileSystem::getFileByteSize(const FilePath& filePath)
{
	return boost::filesystem::file_size(filePath.getPath());
}

TimeStamp FileSystem::getLastWriteTime(const FilePath& filePath)
{
	boost::posix_time::ptime lastWriteTime;
	if (filePath.exists())
	{
		std::time_t t = boost::filesystem::last_write_time(filePath.getPath());
		lastWriteTime = boost::posix_time::from_time_t(t);
		lastWriteTime = boost::date_time::c_local_adjustor<boost::posix_time::ptime>::utc_to_local(
			lastWriteTime);
	}
	return TimeStamp(lastWriteTime);
}

bool FileSystem::remove(const FilePath& path)
{
	boost::system::error_code ec;
	const bool ret = boost::filesystem::remove(path.getPath(), ec);
	path.recheckExists();
	return ret;
}

bool FileSystem::rename(const FilePath& from, const FilePath& to)
{
	if (!from.recheckExists() || to.recheckExists())
	{
		return false;
	}

	boost::filesystem::rename(from.getPath(), to.getPath());
	to.recheckExists();
	return true;
}

bool FileSystem::copyFile(const FilePath& from, const FilePath& to)
{
	if (!from.recheckExists() || to.recheckExists())
	{
		return false;
	}

	boost::filesystem::copy_file(from.getPath(), to.getPath());
	to.recheckExists();
	return true;
}

bool FileSystem::copyDirectory(const FilePath &from, const FilePath &to)
{
	if (!from.recheckExists() || to.recheckExists())
	{
		return false;
	}
	
	boost::filesystem::copy(from.getPath(), to.getPath(), boost::filesystem::copy_options::recursive);
	to.recheckExists();
	return true;
}

void FileSystem::createDirectories(const FilePath& path)
{
	boost::filesystem::create_directories(path.str());
	path.recheckExists();
}

std::vector<FilePath> FileSystem::getDirectSubDirectories(const FilePath& path)
{
	std::vector<FilePath> v;

	if (path.exists() && path.isDirectory())
	{
		for (boost::filesystem::directory_iterator end, dir(path.str()); dir != end; dir++)
		{
			if (boost::filesystem::is_directory(dir->path()))
			{
				v.push_back(FilePath(dir->path().string()));
			}
		}
	}

	return v;
}
