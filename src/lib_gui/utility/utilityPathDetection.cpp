#include "utilityPathDetection.h"

#include "language_packages.h"

#include "utilityApp.h"

#if BUILD_CXX_LANGUAGE_PACKAGE
#	include "CxxFrameworkPathDetector.h"
#	include "CxxHeaderPathDetector.h"
#	include "CxxVs10To14HeaderPathDetector.h"
#	include "CxxVs15ToLatestHeaderPathDetector.h"
#	include "ToolVersionSupport.h"
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
	for (const string &versionRange : VisualStudioVersionSupport::getVersionRanges())
	{
		combinedDetector->addDetector(make_shared<CxxVs15ToLatestHeaderPathDetector>(versionRange));
	}
	// Old Visual Studio versions:

	combinedDetector->addDetector(std::make_shared<CxxVs10To14HeaderPathDetector>(
		CxxVs10To14HeaderPathDetector::VISUAL_STUDIO_2015, false, Platform::Architecture::BITS_32));
	combinedDetector->addDetector(std::make_shared<CxxVs10To14HeaderPathDetector>(
		CxxVs10To14HeaderPathDetector::VISUAL_STUDIO_2015, false, Platform::Architecture::BITS_64));
	combinedDetector->addDetector(std::make_shared<CxxVs10To14HeaderPathDetector>(
		CxxVs10To14HeaderPathDetector::VISUAL_STUDIO_2015, true, Platform::Architecture::BITS_32));
	combinedDetector->addDetector(std::make_shared<CxxVs10To14HeaderPathDetector>(
		CxxVs10To14HeaderPathDetector::VISUAL_STUDIO_2015, true, Platform::Architecture::BITS_64));
	combinedDetector->addDetector(std::make_shared<CxxVs10To14HeaderPathDetector>(
		CxxVs10To14HeaderPathDetector::VISUAL_STUDIO_2013, false, Platform::Architecture::BITS_32));
	combinedDetector->addDetector(std::make_shared<CxxVs10To14HeaderPathDetector>(
		CxxVs10To14HeaderPathDetector::VISUAL_STUDIO_2013, false, Platform::Architecture::BITS_64));
	combinedDetector->addDetector(std::make_shared<CxxVs10To14HeaderPathDetector>(
		CxxVs10To14HeaderPathDetector::VISUAL_STUDIO_2013, true, Platform::Architecture::BITS_32));
	combinedDetector->addDetector(std::make_shared<CxxVs10To14HeaderPathDetector>(
		CxxVs10To14HeaderPathDetector::VISUAL_STUDIO_2013, true, Platform::Architecture::BITS_64));
	combinedDetector->addDetector(std::make_shared<CxxVs10To14HeaderPathDetector>(
		CxxVs10To14HeaderPathDetector::VISUAL_STUDIO_2012, false, Platform::Architecture::BITS_32));
	combinedDetector->addDetector(std::make_shared<CxxVs10To14HeaderPathDetector>(
		CxxVs10To14HeaderPathDetector::VISUAL_STUDIO_2012, false, Platform::Architecture::BITS_64));
	combinedDetector->addDetector(std::make_shared<CxxVs10To14HeaderPathDetector>(
		CxxVs10To14HeaderPathDetector::VISUAL_STUDIO_2012, true, Platform::Architecture::BITS_32));
	combinedDetector->addDetector(std::make_shared<CxxVs10To14HeaderPathDetector>(
		CxxVs10To14HeaderPathDetector::VISUAL_STUDIO_2012, true, Platform::Architecture::BITS_64));
	combinedDetector->addDetector(std::make_shared<CxxVs10To14HeaderPathDetector>(
		CxxVs10To14HeaderPathDetector::VISUAL_STUDIO_2010, false, Platform::Architecture::BITS_32));
	combinedDetector->addDetector(std::make_shared<CxxVs10To14HeaderPathDetector>(
		CxxVs10To14HeaderPathDetector::VISUAL_STUDIO_2010, false, Platform::Architecture::BITS_64));
	combinedDetector->addDetector(std::make_shared<CxxVs10To14HeaderPathDetector>(
		CxxVs10To14HeaderPathDetector::VISUAL_STUDIO_2010, true, Platform::Architecture::BITS_32));
	combinedDetector->addDetector(std::make_shared<CxxVs10To14HeaderPathDetector>(
		CxxVs10To14HeaderPathDetector::VISUAL_STUDIO_2010, true, Platform::Architecture::BITS_64));
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
