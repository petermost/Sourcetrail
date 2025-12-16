#include "utilityPathDetection.h"

#include "language_packages.h"

#include "utilityApp.h"

#if BUILD_CXX_LANGUAGE_PACKAGE
#	include "CxxFrameworkPathDetector.h"
#	include "CxxHeaderPathDetector.h"
#	include "CxxVs17ToLatestHeaderPathDetector.h"
#	include "ToolChain.h"
#endif

#if BUILD_JAVA_LANGUAGE_PACKAGE
#	include "JavaPathDetectorLinuxWindowsMac.h"
#	include "JreSystemLibraryPathDetector.h"
#	include "MavenPathDetectorUnixWindows.h"
#endif

using namespace std;

std::shared_ptr<CombinedPathDetector> utility::getJavaRuntimePathDetector()
{
	std::shared_ptr<CombinedPathDetector> combinedDetector = std::make_shared<CombinedPathDetector>();

#if BUILD_JAVA_LANGUAGE_PACKAGE
	combinedDetector->addDetector(std::make_shared<JavaPathDetectorLinuxWindowsMac>("Java for " + Platform::getName()));
#endif

	return combinedDetector;
}

std::shared_ptr<CombinedPathDetector> utility::getJreSystemLibraryPathsDetector()
{
	std::shared_ptr<CombinedPathDetector> combinedDetector = std::make_shared<CombinedPathDetector>();

#if BUILD_JAVA_LANGUAGE_PACKAGE
	combinedDetector->addDetector(std::make_shared<JreSystemLibraryPathDetector>(
		std::make_shared<JavaPathDetectorLinuxWindowsMac>("Java for " + Platform::getName())));
#endif

	return combinedDetector;
}

std::shared_ptr<CombinedPathDetector> utility::getMavenExecutablePathDetector()
{
	std::shared_ptr<CombinedPathDetector> combinedDetector = std::make_shared<CombinedPathDetector>();

#if BUILD_JAVA_LANGUAGE_PACKAGE
	combinedDetector->addDetector(std::make_shared<MavenPathDetectorUnixWindows>("Maven for " + Platform::getName()));
#endif

	return combinedDetector;
}

std::shared_ptr<CombinedPathDetector> utility::getCxxVsHeaderPathDetector()
{
	std::shared_ptr<CombinedPathDetector> combinedDetector = std::make_shared<CombinedPathDetector>();

	if constexpr (!utility::Platform::isWindows())
	{
		return combinedDetector;
	}

#if BUILD_CXX_LANGUAGE_PACKAGE
	for (const string &versionRange : VisualStudio::getVersionRanges())
	{
		combinedDetector->addDetector(make_shared<CxxVs17ToLatestHeaderPathDetector>(versionRange));
	}
#endif

	return combinedDetector;
}

std::shared_ptr<CombinedPathDetector> utility::getCxxHeaderPathDetector()
{
	std::shared_ptr<CombinedPathDetector> combinedDetector = getCxxVsHeaderPathDetector();
#if BUILD_CXX_LANGUAGE_PACKAGE
	combinedDetector->addDetector(std::make_shared<CxxHeaderPathDetector>("clang"));
	combinedDetector->addDetector(std::make_shared<CxxHeaderPathDetector>("gcc"));
#endif
	return combinedDetector;
}

std::shared_ptr<CombinedPathDetector> utility::getCxxFrameworkPathDetector()
{
	std::shared_ptr<CombinedPathDetector> combinedDetector = std::make_shared<CombinedPathDetector>();
#if BUILD_CXX_LANGUAGE_PACKAGE
	combinedDetector->addDetector(std::make_shared<CxxFrameworkPathDetector>("clang"));
	combinedDetector->addDetector(std::make_shared<CxxFrameworkPathDetector>("gcc"));
#endif
	return combinedDetector;
}
