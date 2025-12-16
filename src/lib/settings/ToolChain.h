#ifndef TOOL_CHAIN_H
#define TOOL_CHAIN_H

#include <string>
#include <vector>

// This is an attempt to centralize the various toolchain information:

class ToolChain { // Just a 'marker' interface
};

class ClangCompiler final : public ToolChain {
	public:
		static constexpr char TOOL_NAME[] = "clang-tool";
		static constexpr char C_LANGUAGE[] = "c";
		static constexpr char CPP_LANGUAGE[] = "c++";

		static std::string verboseOption();

		static std::string stdOption(const std::string &languageVersion);
		static std::string stdCOption(const std::string &version);
		static std::string stdCppOption(const std::string &version);
		static std::string pthreadOption();

		static std::string compileOption();
		static std::string syntaxOnlyOption();

		// MSVC options:

		static std::string msExtensionsOption();
		static std::string msCompatibilityOption();
		static std::string msCompatibilityVersionOption(const std::string &version);

		// Preprocessor:

		static std::string preprocessOption();

		static std::string defineOption(const std::string &nameValue);
		static std::string undefineOption(const std::string &name);

		static std::string includeOption();
		static std::string includeOption(const std::string &directory);

		static std::string systemIncludeOption();
		static std::string systemIncludeOption(const std::string &directory);

		static std::string frameworkIncludeOption();
		static std::string frameworkIncludeOption(const std::string &directory);

		static std::string forceIncludeOption();
		static std::string forceIncludeOption2();
		static std::string forceIncludeOption(const std::string &file);

		static std::string quoteIncludeOption();

		static std::string outputOption();

		static std::string noWarningsOption();

		// Precompiled headers (PCH) options:

		static std::string emitPchOption();
		static std::string includePchOption();
		static std::string allowPchWithCompilerErrors();

		// Other options:

		static std::string errorLimitOption(int limit);
		static std::string exceptionsOption();
		static std::string noDelayedTemplateParsingOption();

		static std::string languageOption();

		static std::string targetOption(const std::string &target);

		// Language version getters:

		static std::string getLatestCppStandard();
		static std::string getLatestCppDraft();
		static std::vector<std::string> getAvailableCppStandards();
		
		static std::string getLatestCStandard();
		static std::string getLatestCDraft();
		static std::vector<std::string> getAvailableCStandards();

		// Environment queries:

		static std::vector<std::string> getAvailableArchTypes();
		static std::vector<std::string> getAvailableVendorTypes();
		static std::vector<std::string> getAvailableOsTypes();
		static std::vector<std::string> getAvailableEnvironmentTypes();
};

class EclipseCompiler final : public ToolChain {
	public:
		static std::string getLatestJavaStandard();
		static std::vector<std::string> getAvailableJavaStandards();
};

class VisualStudio final : public ToolChain {
	public:
		static std::vector<std::string> getVersionRanges();

		static std::string getLatestMsvcVersion();
};

class WindowsSdk final : public ToolChain {
	public:
		static std::vector<std::string> getVersions();
};

void replaceMsvcArguments(std::vector<std::string> *args);

#endif
