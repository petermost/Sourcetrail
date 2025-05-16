#include "CodeblocksTarget.h"

#include "tinyxml.h"

#include "CodeblocksCompiler.h"
#include "utilityString.h"

namespace Codeblocks
{
std::string Target::getXmlElementName()
{
	return "Target";
}

std::shared_ptr<Target> Target::create(const TiXmlElement* element)
{
	if (!element || element->Value() != getXmlElementName())
	{
		return std::shared_ptr<Target>();
	}

	std::shared_ptr<Target> target(new Target());

	{
		const char* value = element->Attribute("title");
		if (!value)
		{
			return std::shared_ptr<Target>();
		}
		target->m_title = utility::decodeFromUtf8(value);
	}

	{
		const TiXmlElement* optionElement = element->FirstChildElement("Option");
		while (optionElement)
		{
			{
				int value = 0;
				if (optionElement->QueryIntAttribute("projectCompilerOptionsRelation", &value) ==
					TIXML_SUCCESS)
				{
					target->m_projectCompilerOptionsRelation = intToEnum<TargetRelationType>(value);
				}
			}
			{
				int value = 0;
				if (optionElement->QueryIntAttribute("projectIncludeDirsRelation", &value) ==
					TIXML_SUCCESS)
				{
					target->m_projectIncludeDirsRelation = intToEnum<TargetRelationType>(value);
				}
			}

			optionElement = optionElement->NextSiblingElement("Option");
		}
	}

	{
		const TiXmlElement* compilerElement = element->FirstChildElement(
			Compiler::getXmlElementName().c_str());
		std::shared_ptr<Compiler> compiler = Compiler::create(compilerElement);
		if (!compiler)
		{
			return std::shared_ptr<Target>();
		}
		target->m_compiler = compiler;
	}

	return target;
}

std::string Target::getTitle() const
{
	return m_title;
}

std::shared_ptr<const Compiler> Target::getCompiler() const
{
	return m_compiler;
}

Target::Target() = default;
}	 // namespace Codeblocks
