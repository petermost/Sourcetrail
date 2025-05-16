#ifndef NAME_HIERARCHY_H
#define NAME_HIERARCHY_H

#include <string>
#include <vector>

#include "NameDelimiterType.h"
#include "NameElement.h"

class NameHierarchy
{
public:
	static std::string serialize(const NameHierarchy& nameHierarchy);
	static std::string serializeRange(const NameHierarchy& nameHierarchy, size_t first, size_t last);
	static NameHierarchy deserialize(const std::string& serializedName);

	NameHierarchy(std::string delimiter);
	NameHierarchy(std::string name, std::string delimiter);
	NameHierarchy(const std::vector<std::string>& names, std::string delimiter);

	NameHierarchy(const NameDelimiterType delimiterType = NAME_DELIMITER_UNKNOWN);
	NameHierarchy(std::string name, const NameDelimiterType delimiterType);
	NameHierarchy(const std::vector<std::string>& names, const NameDelimiterType delimiterType);

	NameHierarchy(const NameHierarchy& other);
	NameHierarchy(NameHierarchy&& other);
	~NameHierarchy();

	const std::string& getDelimiter() const;
	void setDelimiter(std::string delimiter);

	void push(NameElement element);
	void push(std::string name);
	void pop();

	NameElement& back();
	const NameElement& back() const;
	NameElement& operator[](size_t pos);
	const NameElement& operator[](size_t pos) const;

	NameHierarchy& operator=(const NameHierarchy& other);
	NameHierarchy& operator=(NameHierarchy&& other);

	NameHierarchy getRange(size_t first, size_t last) const;

	size_t size() const;

	std::string getQualifiedName() const;
	std::string getQualifiedNameWithSignature() const;
	std::string getRawName() const;
	std::string getRawNameWithSignature() const;
	std::string getRawNameWithSignatureParameters() const;

	bool hasSignature() const;
	NameElement::Signature getSignature() const;

private:
	std::vector<NameElement> m_elements;
	std::string m_delimiter;
};

#endif	  // NAME_ELEMENT_H
