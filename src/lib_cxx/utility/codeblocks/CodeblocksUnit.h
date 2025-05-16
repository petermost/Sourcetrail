#ifndef CODEBLOCKS_UNIT_H
#define CODEBLOCKS_UNIT_H

#include <memory>
#include <set>

#include "CodeblocksCompilerVarType.h"

class FilePath;
class TiXmlElement;

namespace Codeblocks
{
class Unit
{
public:
	static std::string getXmlElementName();
	static std::shared_ptr<Unit> create(const TiXmlElement* element);

	FilePath getCanonicalFilePath(const FilePath& projectFileDirectory) const;
	CompilerVarType getCompilerVar() const;
	bool getCompile() const;
	std::set<std::string> getTargetNames() const;

private:
	Unit();

	std::string m_filename;
	CompilerVarType m_compilerVar = COMPILER_VAR_CPP;
	bool m_compile = true;
	std::set<std::string> m_targetNames;
};
}	 // namespace Codeblocks

#endif	  // CODEBLOCKS_UNIT_H
