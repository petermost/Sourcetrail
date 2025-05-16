#ifndef FILE_PATH_H
#define FILE_PATH_H

#include <boost/filesystem.hpp>
#include <memory>
#include <string>
#include <vector>

class FilePath
{
public:
	static char getEnvironmentVariablePathSeparator();
	static std::string getExecutableExtension();

	FilePath();
	FilePath(const char filePath[]);
	FilePath(const std::string& filePath);
	FilePath(const FilePath& other);
	FilePath(FilePath&& other);
	FilePath(const std::string& filePath, const std::string& base);
	~FilePath();

	const boost::filesystem::path &getPath() const;

	bool empty() const;
	bool exists() const noexcept;
	bool recheckExists() const;
	bool isDirectory() const;
	bool isAbsolute() const;
	bool isValid() const;

	FilePath getParentDirectory() const;

	FilePath& makeAbsolute();
	FilePath getAbsolute() const;
	FilePath& makeCanonical();
	FilePath getCanonical() const;
	FilePath& makeRelativeTo(const FilePath& other);
	FilePath getRelativeTo(const FilePath& other) const;
	FilePath& concatenate(const FilePath& other);
	FilePath getConcatenated(const FilePath& other) const;
	FilePath& concatenate(const char other[]);
	FilePath getConcatenated(const char other[]) const;
	FilePath getLowerCase() const;
	std::vector<FilePath> expandEnvironmentVariables() const;

	bool contains(const FilePath& other) const;

	std::string str() const;
	std::string fileName() const;

	std::string extension() const;
	FilePath withoutExtension() const;
	FilePath replaceExtension(const std::string& extension) const;
	bool hasExtension(const std::vector<std::string>& extensions) const;

	FilePath& operator=(const FilePath& other);
	FilePath& operator=(FilePath&& other);
	bool operator==(const FilePath& other) const;
	bool operator!=(const FilePath& other) const;
	bool operator<(const FilePath& other) const;

private:
	std::unique_ptr<boost::filesystem::path> m_path;

	mutable bool m_exists;
	mutable bool m_checkedExists;
	mutable bool m_isDirectory;
	mutable bool m_checkedIsDirectory;
	mutable bool m_canonicalized;
};

#endif	  // FILE_PATH_H
