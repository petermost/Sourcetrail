#include "ToolChain.h"
#include "language_packages.h"
#include "utility.h"

#if BUILD_CXX_LANGUAGE_PACKAGE
	#include <llvm/Config/llvm-config.h>
#endif

using namespace std;
using namespace string_literals;
using namespace utility;

// Note: We do the '#if LLVM_VERSION_MAJOR ==' check in case we need to support two different clang
// versions in the system build and the vcpkg build!
//
// From llvm-config.h:
// #define LLVM_VERSION_MAJOR 18
// #define LLVM_VERSION_MAJOR 19
//
// C++
// /usr/bin/clang-20 -std=xxx empty.cpp
// vcpkg_installed/x64-arm64-linux-windows-osx-static-md/tools/llvm/clang-18 -std=xxx empty.cpp
//
// Released standards:
// note: use 'c++98' or 'c++03' for 'ISO C++ 1998 with amendments' standard
// note: use 'gnu++98' or 'gnu++03' for 'ISO C++ 1998 with amendments and GNU extensions' standard
// note: use 'c++11' for 'ISO C++ 2011 with amendments' standard
// note: use 'gnu++11' for 'ISO C++ 2011 with amendments and GNU extensions' standard
// note: use 'c++14' for 'ISO C++ 2014 with amendments' standard
// note: use 'gnu++14' for 'ISO C++ 2014 with amendments and GNU extensions' standard
// note: use 'c++17' for 'ISO C++ 2017 with amendments' standard
// note: use 'gnu++17' for 'ISO C++ 2017 with amendments and GNU extensions' standard
// note: use 'c++20' for 'ISO C++ 2020 DIS' standard
// note: use 'gnu++20' for 'ISO C++ 2020 DIS with GNU extensions' standard
// note: use 'c++23' for 'ISO C++ 2023 DIS' standard
// note: use 'gnu++23' for 'ISO C++ 2023 DIS with GNU extensions' standard
//
// Draft standards:
// note: use 'c++2c' or 'c++26' for 'Working draft for C++2c' standard
// note: use 'gnu++2c' or 'gnu++26' for 'Working draft for C++2c with GNU extensions' standard

static vector<string> getReleasedCppStandards()
{
	const vector<string> releasedCppStandards = {
		#ifdef LLVM_VERSION_MAJOR
			#if LLVM_VERSION_MAJOR >= 18
				"c++23"s, "gnu++23"s,
				"c++20"s, "gnu++20"s,
				"c++17"s, "gnu++17"s,
				"c++14"s, "gnu++14"s,
				"c++11"s, "gnu++11"s,
				"c++03"s, "gnu++03"s,
				"c++98"s, "gnu++98"s,
			#endif
		#endif
	};
	return releasedCppStandards;
}

static vector<string> getDraftCppStandards()
{
	const vector<string> draftCppStandards = {
		#ifdef LLVM_VERSION_MAJOR
			#if LLVM_VERSION_MAJOR >= 18
				"c++2c"s, "c++26"s,
				"gnu++2c"s, "gnu++26"s,
			#endif
		#endif
	};
	return draftCppStandards;
}

// C
// /usr/bin/clang-20 -std=xxx empty.c
// vcpkg_installed/x64-arm64-linux-windows-osx-static-md/tools/llvm/clang-18 -std=xxx empty.c
//
// Released standards:
// note: use 'c89', 'c90', or 'iso9899:1990' for 'ISO C 1990' standard
// note: use 'iso9899:199409' for 'ISO C 1990 with amendment 1' standard
// note: use 'gnu89' or 'gnu90' for 'ISO C 1990 with GNU extensions' standard
// note: use 'c99' or 'iso9899:1999' for 'ISO C 1999' standard
// note: use 'gnu99' for 'ISO C 1999 with GNU extensions' standard
// note: use 'c11' or 'iso9899:2011' for 'ISO C 2011' standard
// note: use 'gnu11' for 'ISO C 2011 with GNU extensions' standard
// note: use 'c17', 'iso9899:2017', 'c18', or 'iso9899:2018' for 'ISO C 2017' standard
// note: use 'gnu17' or 'gnu18' for 'ISO C 2017 with GNU extensions' standard
//
// Draft standards:
// note: use 'c23' for 'Working Draft for ISO C23' standard
// note: use 'gnu23' for 'Working Draft for ISO C23 with GNU extensions' standard
// note: use 'c2y' for 'Working Draft for ISO C2y' standard
// note: use 'gnu2y' for 'Working Draft for ISO C2y with GNU extensions' standard

static vector<string> getReleasedCStandards()
{
	const vector<string> releasedCStandards = {
		#ifdef LLVM_VERSION_MAJOR
			#if LLVM_VERSION_MAJOR >= 18
				"c17"s, "gnu17"s,
				"c11"s, "gnu11"s,
				"c99"s, "gnu99"s,
				"c89"s, "gnu89"s,
			#endif
		#endif
	};
	return releasedCStandards;
}

static vector<string> getDraftCStandards()
{
	const vector<string> draftCStandards = {
		#ifdef LLVM_VERSION_MAJOR
			#if LLVM_VERSION_MAJOR == 18 || LLVM_VERSION_MAJOR == 19
				"c23"s, "gnu23"s
			#endif
			#if LLVM_VERSION_MAJOR >= 19
				"c2y"s, "gnu2y"s
			#endif
		#endif
	};
	return draftCStandards;
}

///////////////////////////////////////////////////////////////////////////////
//
// Clang:
//
///////////////////////////////////////////////////////////////////////////////

string ClangCompiler::verboseOption()
{
	return "-v"s;
}

string ClangCompiler::stdOption(const string &languageVersion)
{
	return "-std="s + languageVersion;
}

string ClangCompiler::stdCOption(const string &version)
{
	return stdOption("c"s + version);
}

string ClangCompiler::stdCppOption(const string &version)
{
	return stdOption("c++"s + version);
}

string ClangCompiler::pthreadOption()
{
	return"-pthread"s;
}

string ClangCompiler::compileOption()
{
	// This option signals that no executable is built.

	return "-c"s;
}

string ClangCompiler::syntaxOnlyOption()
{
	return "-fsyntax-only"s;
}


string ClangCompiler::msExtensionsOption()
{
	return "-fms-extensions"s;
}

string ClangCompiler::msCompatibilityOption()
{
	return "-fms-compatibility"s;
}

string ClangCompiler::msCompatibilityVersionOption(const string &version)
{
	return "-fms-compatibility-version="s + version;
}

string ClangCompiler::preprocessOption()
{
	return "-E"s;
}

string ClangCompiler::defineOption(const string &nameValue)
{
	return "-D"s + nameValue;
}

string ClangCompiler::undefineOption(const string &name)
{
	return "-U"s + name;
}

string ClangCompiler::includeOption()
{
	return "-I"s;
}

string ClangCompiler::includeOption(const string &directory)
{
	// -I<dir>, --include-directory <arg>, --include-directory=<arg>

	return includeOption() + directory;
}

string ClangCompiler::forceIncludeOption()
{
	return "-include"s;
}

string ClangCompiler::forceIncludeOption2()
{
	return "--include"s;
}

string ClangCompiler::forceIncludeOption(const string &file)
{
	// -include<file>, --include<file>, --include=<arg>

	return forceIncludeOption() + file;
}

string ClangCompiler::systemIncludeOption()
{
	return "-isystem"s;
}

string ClangCompiler::systemIncludeOption(const string &directory)
{
	// -isystem<directory>

	return systemIncludeOption() + directory;
}

string ClangCompiler::frameworkIncludeOption()
{
	return "-iframework"s;
}

string ClangCompiler::frameworkIncludeOption(const string &directory)
{
	// -iframework<arg>

	return frameworkIncludeOption() + directory;
}

string ClangCompiler::quoteIncludeOption()
{
	return "-iquote"s;
}

string ClangCompiler::outputOption()
{
	return "-o"s;
}

string ClangCompiler::noWarningsOption()
{
	// This option disables all warnings.

	return "-w"s;
}

string ClangCompiler::emitPchOption()
{
	return "-emit-pch"s;
}

string ClangCompiler::includePchOption()
{
	return "-include-pch"s;
}

string ClangCompiler::allowPchWithCompilerErrors()
{
	return "-fallow-pch-with-compiler-errors"s;
}

string ClangCompiler::errorLimitOption(int limit)
{
	// This option tells clang just to continue parsing no matter how manny errors have been thrown.

	return "-ferror-limit="s + to_string(limit);
}

string ClangCompiler::exceptionsOption()
{
	// This option signals that clang should watch out for exception-related code during indexing.

	return "-fexceptions"s;
}

string ClangCompiler::noDelayedTemplateParsingOption()
{
	// This option signals that templates that there should be AST elements for unused template functions as well.

	return "-fno-delayed-template-parsing"s;
}

string ClangCompiler::languageOption()
{
	return "-x"s;
}

string ClangCompiler::targetOption(const string &target)
{
	return "--target="s + target;
}

string ClangCompiler::getLatestCppStandard()
{
	return getReleasedCppStandards()[0];
}

string ClangCompiler::getLatestCppDraft()
{
	return getDraftCppStandards()[0];
}

vector<string> ClangCompiler::getAvailableCppStandards()
{
	return concat(getDraftCppStandards(), getReleasedCppStandards());
}

string ClangCompiler::getLatestCStandard()
{
	return getReleasedCStandards()[0];
}

string ClangCompiler::getLatestCDraft()
{
	return getDraftCStandards()[0];
}

vector<string> ClangCompiler::getAvailableCStandards()
{
	return concat(getDraftCStandards(), getReleasedCStandards());
}

std::vector<std::string> ClangCompiler::getAvailableArchTypes()
{
	// as defined in llvm/lib/Support/Triple.cpp

	return {
		"aarch64",
		"aarch64_be",
		"aarch64_32",
		"arm",
		"armeb",
		"arc",
		"avr",
		"bpfel",
		"bpfeb",
		"hexagon",
		"mips",
		"mipsel",
		"mips64",
		"mips64el",
		"msp430",
		"powerpc64",
		"powerpc64le",
		"powerpc",
		"r600",
		"amdgcn",
		"riscv32",
		"riscv64",
		"sparc",
		"sparcv9",
		"sparcel",
		"s390x",
		"tce",
		"tcele",
		"thumb",
		"thumbeb",
		"i386",
		"x86_64",
		"xcore",
		"nvptx",
		"nvptx64",
		"le32",
		"le64",
		"amdil",
		"amdil64",
		"hsail",
		"hsail64",
		"spir",
		"spir64",
		"kalimba",
		"lanai",
		"shave",
		"wasm32",
		"wasm64",
		"renderscript32",
		"renderscript64",
	};
}

std::vector<std::string> ClangCompiler::getAvailableVendorTypes()
{
	return {
		"unknown",
		"apple",
		"pc",
		"scei",
		"bgp",
		"bgq",
		"fsl",
		"ibm",
		"img",
		"mti",
		"nvidia",
		"csr",
		"myriad",
		"amd",
		"mesa",
		"suse",
		"oe",
	};
}

std::vector<std::string> ClangCompiler::getAvailableOsTypes()
{
	return {
		"unknown",
		"cloudabi",
		"darwin",
		"dragonfly",
		"freebsd",
		"fuchsia",
		"ios",
		"kfreebsd",
		"linux",
		"lv2",
		"macosx",
		"netbsd",
		"openbsd",
		"solaris",
		"windows",
		"haiku",
		"minix",
		"rtems",
		"nacl",
		"cnk",
		"aix",
		"cuda",
		"nvcl",
		"amdhsa",
		"ps4",
		"elfiamcu",
		"tvos",
		"watchos",
		"mesa3d",
		"contiki",
		"amdpal",
		"hermit",
		"hurd",
		"wasi",
		"emscripten",
	};
}

std::vector<std::string> ClangCompiler::getAvailableEnvironmentTypes()
{
	return {
		"unknown",
		"gnu",
		"gnuabin32",
		"gnuabi64",
		"gnueabihf",
		"gnueabi",
		"gnux32",
		"code16",
		"eabi",
		"eabihf",
		"elfv1",
		"elfv2",
		"android",
		"musl",
		"musleabi",
		"musleabihf",
		"msvc",
		"itanium",
		"cygnus",
		"coreclr",
		"simulator",
		"macabi",
	};
}

///////////////////////////////////////////////////////////////////////////////
//
// Eclipse:
//
///////////////////////////////////////////////////////////////////////////////

string EclipseCompiler::getLatestJavaStandard()
{
	return getAvailableJavaStandards()[0];
}

vector<string> EclipseCompiler::getAvailableJavaStandards()
{
	// See org.eclipse.jdt.core.JavaCore for the VERSION_?? values
	// https://github.com/eclipse-jdt/eclipse.jdt.core/blob/master/org.eclipse.jdt.core/model/org/eclipse/jdt/core/JavaCore.java
	const vector<string> availableStandards = {
		"26",  // VERSION_26 = "26"
		"25",  // VERSION_25 = "25"
		"24",  // VERSION_24 = "24"
		"23",  // VERSION_23 = "23"
		"22",  // VERSION_22 = "22"
		"21",  // VERSION_21 = "21"
		"20",  // VERSION_20 = "20"
		"19",  // VERSION_19 = "19"
		"18",  // VERSION_18 = "18"
		"17",  // VERSION_17 = "17"
		"16",  // VERSION_16 = "16"
		"15",  // VERSION_15 = "15"
		"14",  // VERSION_14 = "14"
		"13",  // VERSION_13 = "13"
		"12",  // VERSION_12 = "12"
		"11",  // VERSION_11 = "11"
		"10",  // VERSION_10 = "10"
		"9",   // VERSION_9 = "9"
		"1.8", // VERSION_1_8 = "1.8"
		"1.7", // VERSION_1_7 = "1.7"
		"1.6", // VERSION_1_6 = "1.6"
		"1.5", // VERSION_1_5 = "1.5"
		"1.4", // VERSION_1_4 = "1.4"
		"1.3", // VERSION_1_3 = "1.3"
		"1.2", // VERSION_1_2 = "1.2"
		"1.1"  // VERSION_1_1 = "1.1"
	};
	return availableStandards;
}

///////////////////////////////////////////////////////////////////////////////
//
// Visual Studio:
//
///////////////////////////////////////////////////////////////////////////////

vector<string> VisualStudio::getVersionRanges()
{
	// Version table: https://github.com/microsoft/vswhere/wiki/Versions#release-versions
	// Sorted from newest to oldest:
	const vector<string> releasedVisualStudioVersionRanges = {
		"[18.0, 19.0)"s, // 2026
		"[17.0, 18.0)"s, // 2022
		"[16.0, 17.0)"s, // 2019
		"[15.0, 16.0)"s  // 2017
	};
	return releasedVisualStudioVersionRanges;
}

string VisualStudio::getLatestMsvcVersion()
{
	// Can be found by calling 'cl /?':
	return "19.44"s; // TODO (petermost): 19.50?
}

///////////////////////////////////////////////////////////////////////////////
//
// Windows SDK:
//
///////////////////////////////////////////////////////////////////////////////

vector<string> WindowsSdk::getVersions()
{
	// Adapted from: https://en.wikipedia.org/wiki/Microsoft_Windows_SDK

	const vector<string> sdkVersions = {
		"v10"s,   // Windows Standalone SDK for Windows 10 (Also included in Visual Studio 2015)
		"v8.1A"s, // Included in Visual Studio 2013
		"v8.1"s,  // Windows Software Development Kit (SDK) for Windows 8.1
		"v8.0A"s, // Included in Visual Studio 2012
		"v7.1A"s, // Included in Visual Studio 2012 Update 1 (or later)
		"v7.0A"s  // Included in Visual Studio 2010
	};
	return sdkVersions;
}

///////////////////////////////////////////////////////////////////////////////
//
// MSVC/Clang:
//
///////////////////////////////////////////////////////////////////////////////

static optional<string> getArgumentValue(const string &argument, string_view argumentKey)
{
	return argument.starts_with(argumentKey) ? optional(argument.substr(argumentKey.length())) : nullopt;
}

void replaceMsvcArguments(vector<string> *commandLineArguments)
{
	// Replace/Remove arguments only if these are for the Microsoft C/C++/Resource compiler, otherwise the check for '/' will remove Linux paths:

	if (!commandLineArguments->empty())
	{
		string_view toolName = (*commandLineArguments)[0];
		if (!toolName.ends_with("cl.exe"sv) && !toolName.ends_with("rc.exe"sv))
			return;
	}
	optional<string> argumentValue;

	// - Keep/Replace only those options which are necessary to parse the code correctly
	//
	// From https://learn.microsoft.com/en-us/cpp/build/reference/compiler-options:
	// - All compiler options are case-sensitive.
	// - You may use either a forward slash (/) or a dash (-) to specify a compiler option.

	auto argument = commandLineArguments->begin();
	while (argument != commandLineArguments->end())
	{
		if (argument == commandLineArguments->begin())
			++argument; // skip command

		// Preprocessor symbols:

		else if ((argumentValue = getArgumentValue(*argument, "/D"sv)))
			*argument++ = ClangCompiler::defineOption(*argumentValue);
		else if ((argumentValue = getArgumentValue(*argument, "/U"sv)))
			*argument++ = ClangCompiler::undefineOption(*argumentValue);
		else if ((argumentValue = getArgumentValue(*argument, "/FI"sv)) || (argumentValue = getArgumentValue(*argument, "-FI"sv)))
			*argument++ = ClangCompiler::forceIncludeOption(*argumentValue);

		// Preprocessor include directories:

		else if ((argumentValue = getArgumentValue(*argument, "/I"sv)))
			*argument++ = ClangCompiler::includeOption(*argumentValue);
		else if ((argumentValue = getArgumentValue(*argument, "/external:I"sv)) || (argumentValue = getArgumentValue(*argument, "-external:I"sv)))
			*argument++ = ClangCompiler::systemIncludeOption(*argumentValue);

		// C/C++ language version selection
		// Note: 'latest' and 'preview' must be checked before concrete versions!

		else if (argument->starts_with("/std:c++latest"sv) || argument->starts_with("-std:c++latest"sv))
			*argument++ = ClangCompiler::stdOption(ClangCompiler::getLatestCppStandard());
		else if (argument->starts_with("/std:clatest"sv) || argument->starts_with("-std:clatest"sv))
			*argument++ = ClangCompiler::stdOption(ClangCompiler::getLatestCStandard());

		// Only check for 'preview' and ignore the version:
		// Note: There is no 'c<version>preview', but check it anyway

		else if ((argument->starts_with("/std:c++"sv) || argument->starts_with("-std:c++"sv)) && argument->ends_with("preview"sv))
			*argument++ = ClangCompiler::stdOption(ClangCompiler::getLatestCppDraft());
		else if ((argument->starts_with("/std:c"sv) || argument->starts_with("-std:c"sv)) && argument->ends_with("preview"sv))
			*argument++ = ClangCompiler::stdOption(ClangCompiler::getLatestCDraft());

		else if ((argumentValue = getArgumentValue(*argument, "/std:c++"sv)) || (argumentValue = getArgumentValue(*argument, "-std:c++"sv)))
			*argument++ = ClangCompiler::stdCppOption(*argumentValue);
		else if ((argumentValue = getArgumentValue(*argument, "/std:c"sv)) || (argumentValue = getArgumentValue(*argument, "-std:c"sv)))
			*argument++ = ClangCompiler::stdCOption(*argumentValue);

		// Multithread support:

		else if (getArgumentValue(*argument, "/MD"sv) || getArgumentValue(*argument, "/MT"sv)
			|| getArgumentValue(*argument, "-MD"sv) || getArgumentValue(*argument, "-MT"sv))
		{
			*argument++ = ClangCompiler::pthreadOption();
		}
		// Remove unknown arguments:

		else if (argument->starts_with('/'))
			argument = commandLineArguments->erase(argument);
		else
			++argument;
	}
}
