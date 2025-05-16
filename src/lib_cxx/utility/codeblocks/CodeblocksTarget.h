#ifndef CODEBLOCKS_TARGET_H
#define CODEBLOCKS_TARGET_H

#include <memory>
#include <string>
#include <vector>

#include "CodeblocksTargetRelationType.h"

class TiXmlElement;

namespace Codeblocks
{
class Compiler;

class Target
{
public:
	static std::string getXmlElementName();
	static std::shared_ptr<Target> create(const TiXmlElement* element);

	std::string getTitle() const;
	std::shared_ptr<const Compiler> getCompiler() const;

private:
	Target();

	std::string m_title;
	TargetRelationType m_projectCompilerOptionsRelation = CODEBLOCKS_TARGET_RELATION_TARGET_AFTER_PROJECT;
	TargetRelationType m_projectIncludeDirsRelation = CODEBLOCKS_TARGET_RELATION_TARGET_AFTER_PROJECT;
	std::shared_ptr<Compiler> m_compiler;
};
}	 // namespace Codeblocks

#endif	  // CODEBLOCKS_TARGET_H
