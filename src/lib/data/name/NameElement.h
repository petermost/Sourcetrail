#ifndef NAME_ELEMENT_H
#define NAME_ELEMENT_H

#include <memory>
#include <string>
#include <vector>

class DataType;

class NameElement
{
public:
	class Signature
	{
	public:
		Signature();
		Signature(std::string prefix, std::string postfix);

		std::string qualifyName(const std::string& name) const;
		bool isValid() const;

		const std::string& getPrefix() const;
		const std::string& getPostfix() const;
		std::string getParameterString() const;

	private:
		std::string m_prefix;
		std::string m_postfix;
	};

	NameElement(std::string name);
	NameElement(std::string name, std::string prefix, std::string postfix);
	~NameElement();

	const std::string& getName() const;
	std::string getNameWithSignature() const;
	std::string getNameWithSignatureParameters() const;

	bool hasSignature() const;
	const Signature& getSignature() const;
	void setSignature(std::string prefix, std::string postfix);

private:
	std::string m_name;
	Signature m_signature;
};

#endif	  // NAME_ELEMENT_H
