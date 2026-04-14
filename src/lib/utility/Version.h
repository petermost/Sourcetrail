#ifndef VERSION_H
#define VERSION_H

#include <string>

class Version
{
public:
	static Version getApplicationVersion();

	std::string toDisplayString() const;

private:
	Version(const char versionString[]);

	std::string m_versionString;
};

#endif	  // VERSION_H
