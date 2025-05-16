#include "FilePath.h"

#include <regex>

#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>

#include "logging.h"
#include "utilityApp.h"
#include "utilityString.h"

using namespace utility;
using namespace std;
using namespace std::string_literals;

char FilePath::getEnvironmentVariablePathSeparator()
{
	if constexpr (Platform::isWindows())
		return ';';
	else
		return ':';
}

string FilePath::getExecutableExtension()
{
	if constexpr (Platform::isWindows())
		return ".exe"s;
	else
		return ""s;
}

FilePath::FilePath()
	: m_path(std::make_unique<boost::filesystem::path>(""))
	, m_exists(false)
	, m_checkedExists(false)
	, m_isDirectory(false)
	, m_checkedIsDirectory(false)
	, m_canonicalized(false)
{
}

FilePath::FilePath(const char filePath[])
    : FilePath(string(filePath))
{
}

FilePath::FilePath(const std::string& filePath)
	: m_path(std::make_unique<boost::filesystem::path>(filePath))
	, m_exists(false)
	, m_checkedExists(false)
	, m_isDirectory(false)
	, m_checkedIsDirectory(false)
	, m_canonicalized(false)
{
}

FilePath::FilePath(const FilePath& other)
	: m_path(std::make_unique<boost::filesystem::path>(other.getPath()))
	, m_exists(other.m_exists)
	, m_checkedExists(other.m_checkedExists)
	, m_isDirectory(other.m_isDirectory)
	, m_checkedIsDirectory(other.m_checkedIsDirectory)
	, m_canonicalized(other.m_canonicalized)
{
}

FilePath::FilePath(FilePath&& other)
	: m_path(std::move(other.m_path))
	, m_exists(other.m_exists)
	, m_checkedExists(other.m_checkedExists)
	, m_isDirectory(other.m_isDirectory)
	, m_checkedIsDirectory(other.m_checkedIsDirectory)
	, m_canonicalized(other.m_canonicalized)
{
}

FilePath::FilePath(const std::string& filePath, const std::string& base)
	: m_path(std::make_unique<boost::filesystem::path>(boost::filesystem::absolute(filePath, base)))
	, m_exists(false)
	, m_checkedExists(false)
	, m_isDirectory(false)
	, m_checkedIsDirectory(false)
	, m_canonicalized(false)
{
}

FilePath::~FilePath() = default;

const boost::filesystem::path &FilePath::getPath() const
{
	return *m_path;
}

bool FilePath::empty() const
{
	return m_path->empty();
}

bool FilePath::exists() const noexcept
{
	if (!m_checkedExists)
	{
		m_exists = boost::filesystem::exists(getPath());
		m_checkedExists = true;
	}

	return m_exists;
}

bool FilePath::recheckExists() const
{
	m_checkedExists = false;
	return exists();
}

bool FilePath::isDirectory() const
{
	if (!m_checkedIsDirectory)
	{
		m_isDirectory = boost::filesystem::is_directory(getPath());
		m_checkedIsDirectory = true;
	}

	return m_isDirectory;
}

bool FilePath::isAbsolute() const
{
	return m_path->is_absolute();
}

bool FilePath::isValid() const
{
	boost::filesystem::path::iterator it = m_path->begin();

	if (isAbsolute() && m_path->has_root_path())
	{
		std::string root = m_path->root_path().string();
		std::string current;
		while (current.size() < root.size())
		{
			current += it->string();
			it++;
		}
	}

	for (; it != m_path->end(); ++it)
	{
		if (!boost::filesystem::windows_name(it->string()))
		{
			return false;
		}
	}

	return true;
}

FilePath FilePath::getParentDirectory() const
{
	FilePath parentDirectory(m_path->parent_path().string());

	if (!parentDirectory.empty())
	{
		parentDirectory.m_checkedIsDirectory = true;
		parentDirectory.m_isDirectory = true;

		if (m_checkedExists && m_exists)
		{
			parentDirectory.m_checkedExists = true;
			parentDirectory.m_exists = true;
		}
	}

	return parentDirectory;
}

FilePath& FilePath::makeAbsolute()
{
	m_path = std::make_unique<boost::filesystem::path>(boost::filesystem::absolute(getPath()));
	return *this;
}

FilePath FilePath::getAbsolute() const
{
	FilePath path(*this);
	path.makeAbsolute();
	return path;
}

FilePath& FilePath::makeCanonical()
{
	if (m_canonicalized || !exists())
	{
		return *this;
	}
	try
	{
		boost::filesystem::path canonicalPath = boost::filesystem::canonical(getPath());
		m_path = std::make_unique<boost::filesystem::path>(canonicalPath);
		m_canonicalized = true;
		return *this;
	}
	catch (const boost::filesystem::filesystem_error &e)
	{
		LOG_ERROR_STREAM(<< e.what());
		return *this;
	}
}

FilePath FilePath::getCanonical() const
{
	FilePath path(*this);
	path.makeCanonical();
	return path;
}

std::vector<FilePath> FilePath::expandEnvironmentVariables() const
{
	std::vector<FilePath> paths;
	std::string text = str();

	static std::regex env("\\$\\{([^}]+)\\}|%([^%]+)%");	// ${VARIABLE_NAME} or %VARIABLE_NAME%
	std::smatch match;
	while (std::regex_search(text, match, env))
	{
		const char* s = match[1].matched ? getenv(match[1].str().c_str())
										 : getenv(match[2].str().c_str());
		if (s == nullptr)
		{
			LOG_ERROR_STREAM(<< match[1].str() << " is not an environment variable in: " << text);
			return paths;
		}
		text.replace(match.position(0), match.length(0), s);
	}



	for (const std::string& str: utility::splitToVector(text, getEnvironmentVariablePathSeparator()))
	{
		if (str.size())
		{
			paths.push_back(FilePath(str));
		}
	}

	return paths;
}

FilePath& FilePath::makeRelativeTo(const FilePath& other)
{
	const boost::filesystem::path a = this->getCanonical().getPath();
	const boost::filesystem::path b = other.getCanonical().getPath();

	if (a.root_path() != b.root_path())
	{
		return *this;
	}

	boost::filesystem::path::const_iterator itA = a.begin();
	boost::filesystem::path::const_iterator itB = b.begin();

	while (*itA == *itB && itA != a.end() && itB != b.end())
	{
		itA++;
		itB++;
	}

	boost::filesystem::path r;

	if (itB != b.end())
	{
		if (!boost::filesystem::is_directory(b))
		{
			itB++;
		}

		for (; itB != b.end(); itB++)
		{
			r /= "..";
		}
	}

	for (; itA != a.end(); itA++)
	{
		r /= *itA;
	}

	if (r.empty())
	{
		r = "./";
	}

	m_path = std::make_unique<boost::filesystem::path>(r);
	return *this;
}


FilePath FilePath::getRelativeTo(const FilePath& other) const
{
	FilePath path(*this);
	path.makeRelativeTo(other);
	return path;
}

FilePath& FilePath::concatenate(const FilePath& other)
{
	m_path->operator/=(other.getPath());
	m_exists = false;
	m_checkedExists = false;
	m_isDirectory = false;
	m_checkedIsDirectory = false;
	m_canonicalized = false;

	return *this;
}

FilePath FilePath::getConcatenated(const FilePath& other) const
{
	FilePath path(*this);
	path.concatenate(other);
	return path;
}

FilePath& FilePath::concatenate(const char other[])
{
	m_path->operator/=(other);
	m_exists = false;
	m_checkedExists = false;
	m_isDirectory = false;
	m_checkedIsDirectory = false;
	m_canonicalized = false;

	return *this;
}

FilePath FilePath::getConcatenated(const char other[]) const
{
	FilePath path(*this);
	path.concatenate(other);
	return path;
}

FilePath FilePath::getLowerCase() const
{
	return FilePath(utility::toLowerCase(str()));
}

bool FilePath::contains(const FilePath& other) const
{
	if (!isDirectory())
	{
		return false;
	}

	boost::filesystem::path dir = getPath();
	const std::unique_ptr<boost::filesystem::path>& dir2 = other.m_path;

	if (dir.filename() == ".")
	{
		dir.remove_filename();
	}

	auto it = dir.begin();
	auto it2 = dir2->begin();

	while (it != dir.end())
	{
		if (it2 == dir2->end())
		{
			return false;
		}

		if (*it != *it2)
		{
			return false;
		}

		it++;
		it2++;
	}

	return true;
}

std::string FilePath::str() const
{
	return m_path->generic_string();
}

std::string FilePath::fileName() const
{
	return m_path->filename().generic_string();
}

std::string FilePath::extension() const
{
	return m_path->extension().generic_string();
}

FilePath FilePath::withoutExtension() const
{
	boost::filesystem::path tmpPath(getPath());
	return FilePath(tmpPath.replace_extension().string());
}

FilePath FilePath::replaceExtension(const std::string& extension) const
{
	boost::filesystem::path tmpPath(getPath());
	return FilePath(tmpPath.replace_extension(extension).string());
}

bool FilePath::hasExtension(const std::vector<std::string>& extensions) const
{
	const std::string e = extension();
	for (const std::string& ext: extensions)
	{
		if (e == ext)
		{
			return true;
		}
	}
	return false;
}

FilePath& FilePath::operator=(const FilePath& other)
{
	m_path = std::make_unique<boost::filesystem::path>(other.getPath());
	m_exists = other.m_exists;
	m_checkedExists = other.m_checkedExists;
	m_isDirectory = other.m_isDirectory;
	m_checkedIsDirectory = other.m_checkedIsDirectory;
	m_canonicalized = other.m_canonicalized;
	return *this;
}

FilePath& FilePath::operator=(FilePath&& other)
{
	m_path = std::move(other.m_path);
	m_exists = other.m_exists;
	m_checkedExists = other.m_checkedExists;
	m_isDirectory = other.m_isDirectory;
	m_checkedIsDirectory = other.m_checkedIsDirectory;
	m_canonicalized = other.m_canonicalized;
	return *this;
}

bool FilePath::operator==(const FilePath& other) const
{
	if (exists() && other.exists())
	{
		return boost::filesystem::equivalent(getPath(), other.getPath());
	}

	return m_path->compare(other.getPath()) == 0;
}

bool FilePath::operator!=(const FilePath& other) const
{
	return !(*this == other);
}

bool FilePath::operator<(const FilePath& other) const
{
	return m_path->compare(other.getPath()) < 0;
}
