#include "NameElement.h"

#include "utilityString.h"

NameElement::Signature::Signature() = default;

NameElement::Signature::Signature(std::string prefix, std::string postfix)
	: m_prefix(std::move(prefix)), m_postfix(std::move(postfix))
{
}

std::string NameElement::Signature::qualifyName(const std::string& name) const
{
	if (!isValid())
	{
		return name;
	}

	std::string qualifiedName = m_prefix;
	if (!name.empty())
	{
		if (!m_prefix.empty())
		{
			qualifiedName += " ";
		}
		qualifiedName += name;
	}
	qualifiedName += m_postfix;

	return qualifiedName;
}

bool NameElement::Signature::isValid() const
{
	return !m_prefix.empty() || !m_postfix.empty();
}

const std::string& NameElement::Signature::getPrefix() const
{
	return m_prefix;
}

const std::string& NameElement::Signature::getPostfix() const
{
	return m_postfix;
}

std::string NameElement::Signature::getParameterString() const
{
	if (!m_postfix.empty())
	{
		return utility::substrBeforeLast(m_postfix, ')') + ')';
	}

	return m_postfix;
}

NameElement::NameElement(std::string name): m_name(std::move(name)) {}

NameElement::NameElement(std::string name, std::string prefix, std::string postfix)
	: m_name(std::move(name)), m_signature(std::move(prefix), std::move(postfix))
{
}

NameElement::~NameElement() = default;

const std::string& NameElement::getName() const
{
	return m_name;
}

std::string NameElement::getNameWithSignature() const
{
	return m_signature.qualifyName(m_name);
}

std::string NameElement::getNameWithSignatureParameters() const
{
	return m_name + m_signature.getParameterString();
}

bool NameElement::hasSignature() const
{
	return m_signature.isValid();
}

const NameElement::Signature& NameElement::getSignature() const
{
	return m_signature;
}

void NameElement::setSignature(std::string prefix, std::string postfix)
{
	m_signature = Signature(std::move(prefix), std::move(postfix));
}
