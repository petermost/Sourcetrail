#ifndef TOOLVERSIONSUPPORT_H
#define TOOLVERSIONSUPPORT_H

#include <string>
#include <vector>

// This is an attempt to centralize all the version information which is most likely to change:

class ToolVersionSupport { // Just a 'marker' interface
};

class ClangVersionSupport final : public ToolVersionSupport {
	public:
		static std::string getLatestCppStandard();
		static std::vector<std::string> getAvailableCppStandards();
		
		static std::string getLatestCppDraft();
		
		static std::string getLatestCStandard();
		static std::vector<std::string> getAvailableCStandards();
		
		static std::string getLatestMsCompatibilityVersion();
};

class EclipseVersionSupport final : public ToolVersionSupport {
	public:
		static std::string getLatestJavaStandard();
		static std::vector<std::string> getAvailableJavaStandards();
};

class VisualStudioVersionSupport final : public ToolVersionSupport {
	public:
		static std::vector<std::string> getVersionRanges();
};

class WindowsSdkVersionSupport final : public ToolVersionSupport {
	public:
		static std::vector<std::string> getVersions();
};

#endif
