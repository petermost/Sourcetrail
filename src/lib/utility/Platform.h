#ifndef PLATFORM_H
#define PLATFORM_H

#include <boost/predef.h>
#include <string>

namespace utility
{

class Platform final
{
	public:
		enum class Architecture
		{
			BITS_32,
			BITS_64
		};

		static constexpr bool isLinux()
		{
			// These Unix implementations are similar enough for our purpose, so we don't
			// distinguish them further for now:

			return BOOST_OS_LINUX || BOOST_OS_BSD_FREE;
		}

		static constexpr bool isWindows()
		{
			return BOOST_OS_WINDOWS;
		}

		static constexpr bool isMac()
		{
			return BOOST_OS_MACOS;
		}

		static std::string getName();
		
		static Architecture getArchitecture();
		static std::string getArchitectureName();
		static std::string getArchitectureName(Architecture architecture);

		static std::string getExecutableExtension();
		static std::string getJavaClassPathSeparator();
		static char getEnvironmentVariablePathSeparator();
};

}

#endif // PLATFORM_H
