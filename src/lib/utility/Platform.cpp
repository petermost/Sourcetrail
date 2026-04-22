#include "Platform.h"

namespace utility
{
using namespace std;
using namespace std::string_literals;

string Platform::getName()
{
	static_assert(Platform::isLinux() || Platform::isWindows() || Platform::isMac(), "Unknown platform!");
	static_assert((int)Platform::isLinux() + (int)Platform::isWindows() + (int)Platform::isMac() == 1, "Multiple platforms detected!");

	if constexpr (isLinux())
		return "Linux"s;
	else if constexpr (isWindows())
		return "Windows"s;
	else if constexpr (isMac())
		return "Mac"s;
}

Platform::Architecture Platform::getArchitecture()
{
	// Can't use BOOST_ARCH_WORD_BITS_32/BOOST_ARCH_WORD_BITS_64 because they are *both* defined!
	static_assert(BOOST_ARCH_WORD_BITS == 32 || BOOST_ARCH_WORD_BITS == 64, "Unknown architecture!");

	if constexpr (BOOST_ARCH_WORD_BITS == 32)
		return Architecture::BITS_32;
	else if constexpr (BOOST_ARCH_WORD_BITS == 64)
		return Architecture::BITS_64;
}

string Platform::getArchitectureName()
{
	return getArchitectureName(getArchitecture());
}

string Platform::getArchitectureName(Architecture architecture)
{
	switch (architecture)
	{
		case Architecture::BITS_32:
			return "32 Bit"s;
		case Architecture::BITS_64:
			return "64 Bit"s;
		default:
			return ""s;
	}
}

string Platform::getExecutableExtension()
{
	if constexpr (isWindows())
		return ".exe"s;
	else
		return ""s;
}

string Platform::getJavaClassPathSeparator()
{
	if constexpr (isWindows())
		return ";"s;
	else
		return ":"s;
}

char Platform::getEnvironmentVariablePathSeparator()
{
	if constexpr (isWindows())
		return ';';
	else
		return ':';
}

}
