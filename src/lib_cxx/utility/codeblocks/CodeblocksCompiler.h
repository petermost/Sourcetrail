#ifndef CODEBLOCKS_COMPILER_H
#define CODEBLOCKS_COMPILER_H

#include <memory>
#include <string>
#include <vector>

class TiXmlElement;

class ApplicationSettings;
class FilePath;
class IndexerCommand;
class SourceGroupSettings;
class TextAccess;

namespace Codeblocks
{
class Compiler
{
public:
	static std::string getXmlElementName();
	static std::shared_ptr<Compiler> create(const TiXmlElement* element);

	const std::vector<std::string>& getOptions() const;
	const std::vector<std::string>& getDirectories() const;

private:
	Compiler() = default;

	std::vector<std::string> m_options;
	std::vector<std::string> m_directories;
};
}	 // namespace Codeblocks

#endif	  // CODEBLOCKS_COMPILER_H
