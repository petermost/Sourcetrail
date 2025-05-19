#include "NameHierarchy.h"

#include <sstream>

#include "logging.h"
#include "utilityString.h"

namespace
{
const std::string_view META_DELIMITER = "\tm";
const std::string_view NAME_DELIMITER = "\tn";
const std::string_view PART_DELIMITER = "\ts";
const std::string_view SIGNATURE_DELIMITER = "\tp";
}	 // namespace

std::string NameHierarchy::serialize(const NameHierarchy& nameHierarchy)
{
	return serializeRange(nameHierarchy, 0, nameHierarchy.size());
}

std::string NameHierarchy::serializeRange(const NameHierarchy& nameHierarchy, size_t first, size_t last)
{
	std::stringstream ss;
	ss << nameHierarchy.getDelimiter();
	ss << META_DELIMITER;
	for (size_t i = first; i < last && i < nameHierarchy.size(); i++)
	{
		if (i > 0)
		{
			ss << NAME_DELIMITER;
		}

		ss << nameHierarchy[i].getName() << PART_DELIMITER;
		ss << nameHierarchy[i].getSignature().getPrefix();
		ss << SIGNATURE_DELIMITER;
		ss << nameHierarchy[i].getSignature().getPostfix();
	}
	return ss.str();
}

NameHierarchy NameHierarchy::deserialize(const std::string& serializedName)
{
	size_t mpos = serializedName.find(META_DELIMITER);
	if (mpos == std::string::npos)
	{
		LOG_ERROR("unable to deserialize name hierarchy: " + serializedName);	  // todo: obfuscate
																				  // serializedName!
		return NameHierarchy(NAME_DELIMITER_UNKNOWN);
	}

	NameHierarchy nameHierarchy(serializedName.substr(0, mpos));

	size_t npos = mpos + META_DELIMITER.size();
	while (npos != std::string::npos && npos < serializedName.size())
	{
		// name
		size_t spos = serializedName.find(PART_DELIMITER, npos);
		if (spos == std::string::npos)
		{
			LOG_ERROR(
				"unable to deserialize name hierarchy: " +
				serializedName);	// todo: obfuscate serializedName!
			return NameHierarchy(NAME_DELIMITER_UNKNOWN);
		}

		std::string name = serializedName.substr(npos, spos - npos);
		spos += PART_DELIMITER.size();

		// signature
		size_t ppos = serializedName.find(SIGNATURE_DELIMITER, spos);
		if (ppos == std::string::npos)
		{
			LOG_ERROR(
				"unable to deserialize name hierarchy: " +
				serializedName);	// todo: obfuscate serializedName!
			return NameHierarchy(NAME_DELIMITER_UNKNOWN);
		}

		std::string prefix = serializedName.substr(spos, ppos - spos);
		ppos += SIGNATURE_DELIMITER.size();

		std::string postfix;
		npos = serializedName.find(NAME_DELIMITER, ppos);
		if (npos == std::string::npos)
		{
			postfix = serializedName.substr(ppos, std::string::npos);
		}
		else
		{
			postfix = serializedName.substr(ppos, npos - ppos);
			npos += NAME_DELIMITER.size();
		}

		nameHierarchy.push(NameElement(std::move(name), std::move(prefix), std::move(postfix)));
	}

	// TODO: replace duplicate main definition fix with better solution
	if (nameHierarchy.size() == 1 && nameHierarchy.back().hasSignature() &&
		!nameHierarchy.back().getName().empty() && nameHierarchy.back().getName()[0] == '.' &&
		utility::isPrefix(".:main:.", nameHierarchy.back().getName()))
	{
		NameElement::Signature sig = nameHierarchy.back().getSignature();
		nameHierarchy.pop();
		nameHierarchy.push(NameElement("main", sig.getPrefix(), sig.getPostfix()));
	}

	return nameHierarchy;
}

const std::string& NameHierarchy::getDelimiter() const
{
	return m_delimiter;
}

void NameHierarchy::setDelimiter(std::string delimiter)
{
	m_delimiter = std::move(delimiter);
}

NameHierarchy::NameHierarchy(std::string delimiter): m_delimiter(std::move(delimiter)) {}

NameHierarchy::NameHierarchy(const std::vector<std::string>& names, std::string delimiter)
	: m_delimiter(std::move(delimiter))
{
	for (const std::string& name: names)
	{
		push(name);
	}
}

NameHierarchy::NameHierarchy(std::string name, std::string delimiter)
	: m_delimiter(std::move(delimiter))
{
	push(std::move(name));
}

NameHierarchy::NameHierarchy(const NameDelimiterType delimiterType)
	: NameHierarchy(nameDelimiterTypeToString(delimiterType))
{
}

NameHierarchy::NameHierarchy(std::string name, const NameDelimiterType delimiterType)
	: NameHierarchy(name, nameDelimiterTypeToString(delimiterType))
{
}

NameHierarchy::NameHierarchy(const std::vector<std::string>& names, const NameDelimiterType delimiterType)
	: NameHierarchy(names, nameDelimiterTypeToString(delimiterType))
{
}

NameHierarchy::NameHierarchy(const NameHierarchy& other) = default;

NameHierarchy::NameHierarchy(NameHierarchy&& other)
	: m_elements(std::move(other.m_elements)), m_delimiter(std::move(other.m_delimiter))
{
}

NameHierarchy::~NameHierarchy() = default;

void NameHierarchy::push(NameElement element)
{
	m_elements.emplace_back(std::move(element));
}

void NameHierarchy::push(std::string name)
{
	m_elements.emplace_back(std::move(name));
}

void NameHierarchy::pop()
{
	m_elements.pop_back();
}

NameElement& NameHierarchy::back()
{
	return m_elements.back();
}

const NameElement& NameHierarchy::back() const
{
	return m_elements.back();
}

NameElement& NameHierarchy::operator[](size_t pos)
{
	return m_elements[pos];
}

const NameElement& NameHierarchy::operator[](size_t pos) const
{
	return m_elements[pos];
}

NameHierarchy& NameHierarchy::operator=(const NameHierarchy& other) = default;

NameHierarchy& NameHierarchy::operator=(NameHierarchy&& other) = default;

NameHierarchy NameHierarchy::getRange(size_t first, size_t last) const
{
	NameHierarchy hierarchy(m_delimiter);

	for (size_t i = first; i < last; i++)
	{
		hierarchy.push(m_elements[i]);
	}

	return hierarchy;
}

size_t NameHierarchy::size() const
{
	return m_elements.size();
}

std::string NameHierarchy::getQualifiedName() const
{
	std::stringstream ss;
	for (size_t i = 0; i < m_elements.size(); i++)
	{
		if (i > 0)
		{
			ss << m_delimiter;
		}
		ss << m_elements[i].getName();
	}
	return ss.str();
}

std::string NameHierarchy::getQualifiedNameWithSignature() const
{
	std::string name = getQualifiedName();
	if (m_elements.size())
	{
		name = m_elements.back().getSignature().qualifyName(
			name);	  // todo: use separator for signature!
	}
	return name;
}

std::string NameHierarchy::getRawName() const
{
	if (m_elements.size())
	{
		return m_elements.back().getName();
	}
	return "";
}

std::string NameHierarchy::getRawNameWithSignature() const
{
	if (m_elements.size())
	{
		return m_elements.back().getNameWithSignature();
	}
	return "";
}

std::string NameHierarchy::getRawNameWithSignatureParameters() const
{
	if (m_elements.size())
	{
		return m_elements.back().getNameWithSignatureParameters();
	}
	return "";
}

bool NameHierarchy::hasSignature() const
{
	if (m_elements.size())
	{
		return m_elements.back().hasSignature();
	}

	return false;
}

NameElement::Signature NameHierarchy::getSignature() const
{
	if (m_elements.size())
	{
		return m_elements.back().getSignature();	// todo: use separator for signature!
	}

	return NameElement::Signature();
}
