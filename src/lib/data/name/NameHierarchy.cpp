#include "NameHierarchy.h"

#include "logging.h"
#include "utilityMainFunction.h"

#include <sstream>

using namespace std;

static const std::string_view META_DELIMITER      = "\tm";
static const std::string_view NAME_DELIMITER      = "\tn";
static const std::string_view PART_DELIMITER      = "\ts";
static const std::string_view SIGNATURE_DELIMITER = "\tp";

std::string NameHierarchy::serialize(const NameHierarchy &nameHierarchy)
{
	return serializeRange(nameHierarchy, 0, nameHierarchy.size());
}

std::string NameHierarchy::serializeRange(const NameHierarchy &nameHierarchy, size_t first, size_t last)
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

NameHierarchy NameHierarchy::deserialize(const std::string &serializedName)
{
	size_t mpos = serializedName.find(META_DELIMITER);
	if (mpos == std::string::npos)
	{
		LOG_ERROR("unable to deserialize name hierarchy: " + serializedName);
		return NameHierarchy(NameDelimiterType::UNKNOWN);
	}

	NameHierarchy nameHierarchy(serializedName.substr(0, mpos));

	size_t npos = mpos + META_DELIMITER.size();
	while (npos < serializedName.size())
	{
		// name
		size_t spos = serializedName.find(PART_DELIMITER, npos);
		if (spos == std::string::npos)
		{
			LOG_ERROR("unable to deserialize name hierarchy: " + serializedName);
			return NameHierarchy(NameDelimiterType::UNKNOWN);
		}

		std::string name = serializedName.substr(npos, spos - npos);
		spos += PART_DELIMITER.size();

		// signature
		size_t ppos = serializedName.find(SIGNATURE_DELIMITER, spos);
		if (ppos == std::string::npos)
		{
			LOG_ERROR("unable to deserialize name hierarchy: " + serializedName);
			return NameHierarchy(NameDelimiterType::UNKNOWN);
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

	if (isUniquifiedMainFunction(nameHierarchy))
		deuniquifyMainFunction(&nameHierarchy);

	return nameHierarchy;
}


NameHierarchy::NameHierarchy(std::string delimiter)
	: m_delimiter(std::move(delimiter))
{
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

void NameHierarchy::setDelimiter(std::string delimiter)
{
	m_delimiter = std::move(delimiter);
}

const std::string &NameHierarchy::getDelimiter() const
{
	return m_delimiter;
}

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

NameElement &NameHierarchy::back()
{
	return m_elements.back();
}

const NameElement &NameHierarchy::back() const
{
	return m_elements.back();
}

const NameElement &NameHierarchy::operator[](size_t pos) const
{
	return m_elements[pos];
}

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
	if (m_elements.size() != 0)
	{
		name = m_elements.back().getSignature().qualifyName(name); // todo: use separator for signature!
	}
	return name;
}

std::string NameHierarchy::getRawName() const
{
	if (m_elements.size() != 0)
	{
		return m_elements.back().getName();
	}
	return "";
}

std::string NameHierarchy::getRawNameWithSignature() const
{
	if (m_elements.size() != 0)
	{
		return m_elements.back().getNameWithSignature();
	}
	return "";
}

std::string NameHierarchy::getRawNameWithSignatureParameters() const
{
	if (m_elements.size() != 0)
	{
		return m_elements.back().getNameWithSignatureParameters();
	}
	return "";
}

bool NameHierarchy::hasSignature() const
{
	if (m_elements.size() != 0)
	{
		return m_elements.back().hasSignature();
	}

	return false;
}

NameElement::Signature NameHierarchy::getSignature() const
{
	if (m_elements.size() != 0)
	{
		return m_elements.back().getSignature(); // todo: use separator for signature!
	}

	return NameElement::Signature();
}
