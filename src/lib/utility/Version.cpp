#include "Version.h"
#include "productVersion.h"

Version Version::getApplicationVersion()
{
	return Version(PRODUCT_VERSION_STRING);
}

Version::Version(const char versionString[])
	: m_versionString(versionString)
{
}

std::string Version::toDisplayString() const
{
	return m_versionString;
}
