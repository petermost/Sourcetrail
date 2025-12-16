#include "Catch2.hpp"

#include <fstream>

#include "language_packages.h"

#include "AppPath.h"
#include "Application.h"
#include "ApplicationSettings.h"
#include "FileSystem.h"
#include "IndexerCommandCustom.h"
#include "Platform.h"
#include "ProjectSettings.h"
#include "SourceGroupCustomCommand.h"
#include "SourceGroupSettingsCustomCommand.h"
#include "TextAccess.h"
#include "Version.h"
#include "utilityPathDetection.h"
#include "utilityString.h"

#if BUILD_CXX_LANGUAGE_PACKAGE
#	include "IndexerCommandCxx.h"
#	include "SourceGroupCxxCdb.h"
#	include "SourceGroupCxxCodeblocks.h"
#	include "SourceGroupCxxEmpty.h"
#	include "SourceGroupSettingsCEmpty.h"
#	include "SourceGroupSettingsCppEmpty.h"
#	include "SourceGroupSettingsCxxCdb.h"
#	include "SourceGroupSettingsCxxCodeblocks.h"
#endif	  // BUILD_CXX_LANGUAGE_PACKAGE

#if BUILD_JAVA_LANGUAGE_PACKAGE
#	include "IndexerCommandJava.h"
#	include "JavaEnvironmentFactory.h"
#	include "SourceGroupJavaEmpty.h"
#	include "SourceGroupJavaGradle.h"
#	include "SourceGroupJavaMaven.h"
#	include "SourceGroupSettingsJavaEmpty.h"
#	include "SourceGroupSettingsJavaGradle.h"
#	include "SourceGroupSettingsJavaMaven.h"
#	include "utilityJava.h"
#endif	  // BUILD_JAVA_LANGUAGE_PACKAGE

using namespace std;
using namespace utility;

namespace
{
const bool updateExpectedOutput = false;

// Some tests assume that the shared data directory is empty. Make it easier to clear/reset it
// with a RAII wrapper:
class SharedDataDirectorySwitcher {
public:
	SharedDataDirectorySwitcher(const FilePath &newDirectory)
	{
		m_oldDirectory = AppPath::getSharedDataDirectoryPath();
		
		AppPath::setSharedDataDirectoryPath(newDirectory);
	}
	
	~SharedDataDirectorySwitcher()
	{
		AppPath::setSharedDataDirectoryPath(m_oldDirectory);
	}
private:
	FilePath m_oldDirectory;
};

static FilePath getInputDirectoryPath(const std::string& projectName)
{
	return FilePath("data/SourceGroupTestSuite/" + projectName + "/input")
		.makeAbsolute()
		.makeCanonical();
}

static FilePath getOutputDirectoryPath(const std::string& projectName)
{
	return FilePath("data/SourceGroupTestSuite/" + projectName + "/expected_output")
		.makeAbsolute()
		.makeCanonical();
}

#if BUILD_JAVA_LANGUAGE_PACKAGE
std::string setupJavaEnvironmentFactory()
{
	return utility::prepareJavaEnvironment(FilePath("../app/data/java/"));
}
#endif	  // BUILD_JAVA_LANGUAGE_PACKAGE

#if BUILD_CXX_LANGUAGE_PACKAGE
std::string indexerCommandCxxToString(
	std::shared_ptr<const IndexerCommandCxx> indexerCommand, const FilePath& baseDirectory)
{
	std::string result;
	result += "SourceFilePath: \"" +
		indexerCommand->getSourceFilePath().getRelativeTo(baseDirectory).str() + "\"\n";
	for (const FilePath& indexedPath: indexerCommand->getIndexedPaths())
	{
		result += "\tIndexedPath: \"" + indexedPath.getRelativeTo(baseDirectory).str() + "\"\n";
	}
	for (std::string compilerFlag: indexerCommand->getCompilerFlags())
	{
		FilePath flagAsPath(compilerFlag);
		if (flagAsPath.exists())
		{
			compilerFlag = flagAsPath.getRelativeTo(baseDirectory).str();
		}
		result += "\tCompilerFlag: \"" + compilerFlag + "\"\n";
	}
	for (const FilePathFilter& filter: indexerCommand->getExcludeFilters())
	{
		result += "\tExcludeFilter: \"" + filter.str() + "\"\n";
	}
	return result;
}
#endif	  // BUILD_CXX_LANGUAGE_PACKAGE

#if BUILD_JAVA_LANGUAGE_PACKAGE
std::string indexerCommandJavaToString(
	std::shared_ptr<const IndexerCommandJava> indexerCommand, const FilePath& baseDirectory)
{
	std::string result;
	result += "SourceFilePath: \"" +
		indexerCommand->getSourceFilePath().getRelativeTo(baseDirectory).str() + "\"\n";
	result += "\tLanguageStandard: \"" + indexerCommand->getLanguageStandard() + "\"\n";
	for (const FilePath& classPathItem: indexerCommand->getClassPath())
	{
		result += "\tClassPathItem: \"" + classPathItem.getRelativeTo(baseDirectory).str() +
			"\"\n";
	}
	return result;
}
#endif	  // BUILD_JAVA_LANGUAGE_PACKAGE

std::string indexerCommandCustomToString(
	std::shared_ptr<const IndexerCommandCustom> indexerCommand, const FilePath& baseDirectory)
{
	std::string result;
	result += "IndexerCommandCustom\n";
	result += "\tSourceFilePath: \"" +
		indexerCommand->getSourceFilePath().getRelativeTo(baseDirectory).str() + "\"\n";
	result += "\tCustom Command: \"" + indexerCommand->getCommand() + "\"\n";
	result += "\tArguments:\n";
	for (const std::string& argument: indexerCommand->getArguments())
	{
		result += "\t\t\"" + argument + "\"\n";
	}
	return result;
}

std::string indexerCommandToString(
	std::shared_ptr<IndexerCommand> indexerCommand, const FilePath& baseDirectory)
{
	if (indexerCommand)
	{
#if BUILD_CXX_LANGUAGE_PACKAGE
		if (std::shared_ptr<const IndexerCommandCxx> indexerCommandCxx =
				std::dynamic_pointer_cast<const IndexerCommandCxx>(indexerCommand))
		{
			return indexerCommandCxxToString(indexerCommandCxx, baseDirectory);
		}
#endif	  // BUILD_CXX_LANGUAGE_PACKAGE
#if BUILD_JAVA_LANGUAGE_PACKAGE
		if (std::shared_ptr<const IndexerCommandJava> indexerCommandJava =
				std::dynamic_pointer_cast<const IndexerCommandJava>(indexerCommand))
		{
			return indexerCommandJavaToString(indexerCommandJava, baseDirectory);
		}
#endif	  // BUILD_JAVA_LANGUAGE_PACKAGE
		if (std::shared_ptr<const IndexerCommandCustom> indexerCommandCustom =
				std::dynamic_pointer_cast<const IndexerCommandCustom>(indexerCommand))
		{
			return indexerCommandCustomToString(indexerCommandCustom, baseDirectory);
		}
		return "Unsupported indexer command type: " + indexerCommandTypeToString(indexerCommand->getIndexerCommandType());
	}
	return "No IndexerCommand provided.";
}

std::shared_ptr<TextAccess> generateExpectedOutput(
	std::string projectName, std::shared_ptr<const SourceGroup> sourceGroup)
{
	const FilePath projectDataRoot = getInputDirectoryPath(projectName).makeAbsolute();

	RefreshInfo info;
	info.filesToIndex = sourceGroup->getAllSourceFilePaths();
	std::vector<std::shared_ptr<IndexerCommand>> indexerCommands = sourceGroup->getIndexerCommands(
		info);

	std::sort(
		indexerCommands.begin(),
		indexerCommands.end(),
		[](std::shared_ptr<IndexerCommand> a, std::shared_ptr<IndexerCommand> b) {
			return a->getSourceFilePath().str() < b->getSourceFilePath().str();
		});

	std::string outputString;
	for (const std::shared_ptr<IndexerCommand> &indexerCommand : indexerCommands) {
		outputString += indexerCommandToString(indexerCommand, projectDataRoot);
	}

	return TextAccess::createFromString(outputString);
}

std::string getOutputFilename()
{
	if constexpr (Platform::isWindows())
		return "output_windows.txt";
	else
		return "output_unix.txt";

}
void generateAndCompareExpectedOutput(
	std::string projectName, std::shared_ptr<const SourceGroup> sourceGroup)
{
	const std::shared_ptr<const TextAccess> output = generateExpectedOutput(projectName, sourceGroup);
	const std::string expectedOutputFileName = getOutputFilename();

	const FilePath expectedOutputFilePath =
		getOutputDirectoryPath(projectName).concatenate(expectedOutputFileName);
	if (updateExpectedOutput || !expectedOutputFilePath.exists())
	{
		std::ofstream expectedOutputFile;
		expectedOutputFile.open(expectedOutputFilePath.str());
		expectedOutputFile << output->getText();
		expectedOutputFile.close();
	}
	else
	{
		const std::shared_ptr<const TextAccess> expectedOutput = TextAccess::createFromFile(
			expectedOutputFilePath);
		REQUIRE_MESSAGE(
			("Output does not match the expected line count for project \"" +
			 projectName) + "\". Output was: " + output->getText(),
			expectedOutput->getLineCount() == output->getLineCount());
		if (expectedOutput->getLineCount() == output->getLineCount())
		{
			for (unsigned int i = 1; i <= expectedOutput->getLineCount(); i++)
			{
				REQUIRE(expectedOutput->getLine(i) == output->getLine(i));
			}
		}
	}
}
}	 // namespace

#if BUILD_JAVA_LANGUAGE_PACKAGE

TEST_CASE("finds all jar dependencies")
{
	for (const std::string& jarName: utility::getRequiredJarNames())
	{
		FilePath jarPath = FilePath("../app/data/java/lib/").concatenate(jarName);
		REQUIRE_MESSAGE("Jar dependency path does not exist: " + jarPath.str(), jarPath.exists());
	}
}

TEST_CASE("can setup environment factory")
{
	std::vector<FilePath> javaPaths = utility::getJavaRuntimePathDetector()->getPaths();
	if (!javaPaths.empty())
	{
		ApplicationSettings::getInstance()->setJavaPath(javaPaths[0]);
	}
	const std::string errorString = setupJavaEnvironmentFactory();

	REQUIRE("" == errorString);

	// if this one fails, maybe your java_path in the test settings is wrong.
	REQUIRE(JavaEnvironmentFactory::getInstance().use_count() >= 1);
}

#endif	  // BUILD_JAVA_LANGUAGE_PACKAGE

TEST_CASE("can create application instance")
{
	// required to query in SourceGroup for dialog view... this is not a very elegant solution.
	// should be refactored to pass dialog view to SourceGroup on creation.
	Application::createInstance(Version(), nullptr, nullptr);
	REQUIRE(Application::getInstance().use_count() >= 1);
}

#if BUILD_CXX_LANGUAGE_PACKAGE
TEST_CASE("source group cxx c empty generates expected output")
{
	SharedDataDirectorySwitcher sharedDataDirectorySwitcher((FilePath()));

	const std::string projectName = "cxx_c_empty";

	ProjectSettings projectSettings;
	projectSettings.setProjectFilePath("non_existent_project", getInputDirectoryPath(projectName));

	std::shared_ptr<SourceGroupSettingsCEmpty> sourceGroupSettings =
		std::make_shared<SourceGroupSettingsCEmpty>("fake_id", &projectSettings);
	sourceGroupSettings->setSourcePaths({getInputDirectoryPath(projectName).concatenate("src")});
	sourceGroupSettings->setSourceExtensions({".c"});
	sourceGroupSettings->setExcludeFilterStrings({"**/excluded/**"});
	sourceGroupSettings->setTargetOptionsEnabled(true);
	sourceGroupSettings->setTargetArch("test_arch");
	sourceGroupSettings->setTargetVendor("test_vendor");
	sourceGroupSettings->setTargetSys("test_sys");
	sourceGroupSettings->setTargetAbi("test_abi");
	sourceGroupSettings->setCStandard("c11");
	sourceGroupSettings->setHeaderSearchPaths(
		{getInputDirectoryPath(projectName).concatenate("header_search/local")});
	sourceGroupSettings->setFrameworkSearchPaths(
		{getInputDirectoryPath(projectName).concatenate("framework_search/local")});
	sourceGroupSettings->setCompilerFlags({"-local-flag"});

	std::shared_ptr<ApplicationSettings> applicationSettings = ApplicationSettings::getInstance();

	std::vector<FilePath> storedHeaderSearchPaths = applicationSettings->getHeaderSearchPaths();
	std::vector<FilePath> storedFrameworkSearchPaths = applicationSettings->getFrameworkSearchPaths();

	applicationSettings->setHeaderSearchPaths({FilePath("test/header/search/path")});
	applicationSettings->setFrameworkSearchPaths({FilePath("test/framework/search/path")});

	generateAndCompareExpectedOutput(
		projectName, std::make_shared<SourceGroupCxxEmpty>(sourceGroupSettings));

	applicationSettings->setHeaderSearchPaths(storedHeaderSearchPaths);
	applicationSettings->setFrameworkSearchPaths(storedFrameworkSearchPaths);
}

TEST_CASE("source group cxx cpp empty generates expected output")
{
	SharedDataDirectorySwitcher sharedDataDirectorySwitcher((FilePath()));
	
	const std::string projectName = "cxx_cpp_empty";

	ProjectSettings projectSettings;
	projectSettings.setProjectFilePath("non_existent_project", getInputDirectoryPath(projectName));

	std::shared_ptr<SourceGroupSettingsCppEmpty> sourceGroupSettings =
		std::make_shared<SourceGroupSettingsCppEmpty>("fake_id", &projectSettings);
	sourceGroupSettings->setSourcePaths({getInputDirectoryPath(projectName).concatenate("/src")});
	sourceGroupSettings->setSourceExtensions({".cpp"});
	sourceGroupSettings->setExcludeFilterStrings({"**/excluded/**"});
	sourceGroupSettings->setTargetOptionsEnabled(true);
	sourceGroupSettings->setTargetArch("test_arch");
	sourceGroupSettings->setTargetVendor("test_vendor");
	sourceGroupSettings->setTargetSys("test_sys");
	sourceGroupSettings->setTargetAbi("test_abi");
	sourceGroupSettings->setCppStandard("c++11");
	sourceGroupSettings->setHeaderSearchPaths(
		{getInputDirectoryPath(projectName).concatenate("header_search/local")});
	sourceGroupSettings->setFrameworkSearchPaths(
		{getInputDirectoryPath(projectName).concatenate("framework_search/local")});
	sourceGroupSettings->setCompilerFlags({"-local-flag"});

	std::shared_ptr<ApplicationSettings> applicationSettings = ApplicationSettings::getInstance();

	std::vector<FilePath> storedHeaderSearchPaths = applicationSettings->getHeaderSearchPaths();
	std::vector<FilePath> storedFrameworkSearchPaths = applicationSettings->getFrameworkSearchPaths();

	applicationSettings->setHeaderSearchPaths({FilePath("test/header/search/path")});
	applicationSettings->setFrameworkSearchPaths({FilePath("test/framework/search/path")});

	generateAndCompareExpectedOutput(
		projectName, std::make_shared<SourceGroupCxxEmpty>(sourceGroupSettings));

	applicationSettings->setHeaderSearchPaths(storedHeaderSearchPaths);
	applicationSettings->setFrameworkSearchPaths(storedFrameworkSearchPaths);
}

TEST_CASE("source group cxx codeblocks generates expected output")
{
	SharedDataDirectorySwitcher sharedDataDirectorySwitcher((FilePath()));

	const std::string projectName = "cxx_codeblocks";
	const FilePath cbpPath = getInputDirectoryPath(projectName).concatenate("project.cbp");
	const FilePath sourceCbpPath =
		getInputDirectoryPath(projectName).concatenate("project.cbp.in");

	FileSystem::remove(cbpPath);

	{
		std::ofstream fileStream;
		fileStream.open(cbpPath.str(), std::ios::app);
		fileStream << utility::replace(
			TextAccess::createFromFile(sourceCbpPath)->getText(),
			"<source_path>",
			getInputDirectoryPath(projectName).concatenate("src").getAbsolute().str());
		fileStream.close();
	}

	ProjectSettings projectSettings;
	projectSettings.setProjectFilePath("non_existent_project", getInputDirectoryPath(projectName));

	std::shared_ptr<SourceGroupSettingsCxxCodeblocks> sourceGroupSettings =
		std::make_shared<SourceGroupSettingsCxxCodeblocks>("fake_id", &projectSettings);
	sourceGroupSettings->setCodeblocksProjectPath(cbpPath);
	sourceGroupSettings->setCppStandard("c++11");
	sourceGroupSettings->setCStandard("c11");
	sourceGroupSettings->setExcludeFilterStrings({"**/excluded/**"});
	sourceGroupSettings->setIndexedHeaderPaths({FilePath("test/indexed/header/path")});
	sourceGroupSettings->setSourceExtensions({".cpp", ".c"});
	sourceGroupSettings->setHeaderSearchPaths(
		{getInputDirectoryPath(projectName).concatenate("header_search/local")});
	sourceGroupSettings->setFrameworkSearchPaths(
		{getInputDirectoryPath(projectName).concatenate("framework_search/local")});
	sourceGroupSettings->setCompilerFlags({"-local-flag"});

	std::shared_ptr<ApplicationSettings> applicationSettings = ApplicationSettings::getInstance();

	std::vector<FilePath> storedHeaderSearchPaths = applicationSettings->getHeaderSearchPaths();
	std::vector<FilePath> storedFrameworkSearchPaths = applicationSettings->getFrameworkSearchPaths();

	applicationSettings->setHeaderSearchPaths({FilePath("test/header/search/path")});
	applicationSettings->setFrameworkSearchPaths({FilePath("test/framework/search/path")});

	generateAndCompareExpectedOutput(
		projectName, std::make_shared<SourceGroupCxxCodeblocks>(sourceGroupSettings));

	applicationSettings->setHeaderSearchPaths(storedHeaderSearchPaths);
	applicationSettings->setFrameworkSearchPaths(storedFrameworkSearchPaths);

	FileSystem::remove(cbpPath);
}

TEST_CASE("source group cxx cdb generates expected output")
{
	SharedDataDirectorySwitcher sharedDataDirectorySwitcher((FilePath()));

	const std::string projectName = "cxx_cdb";

	ProjectSettings projectSettings;
	projectSettings.setProjectFilePath("non_existent_project", getInputDirectoryPath(projectName));

	std::shared_ptr<SourceGroupSettingsCxxCdb> sourceGroupSettings =
		std::make_shared<SourceGroupSettingsCxxCdb>("fake_id", &projectSettings);
	sourceGroupSettings->setIndexedHeaderPaths({FilePath("test/indexed/header/path")});
	sourceGroupSettings->setCompilationDatabasePath(
		getInputDirectoryPath(projectName).concatenate("compile_commands.json"));
	sourceGroupSettings->setExcludeFilterStrings({"**/excluded/**"});
	sourceGroupSettings->setHeaderSearchPaths(
		{getInputDirectoryPath(projectName).concatenate("header_search/local")});
	sourceGroupSettings->setFrameworkSearchPaths(
		{getInputDirectoryPath(projectName).concatenate("framework_search/local")});
	sourceGroupSettings->setCompilerFlags({"-local-flag"});

	std::shared_ptr<ApplicationSettings> applicationSettings = ApplicationSettings::getInstance();

	std::vector<FilePath> storedHeaderSearchPaths = applicationSettings->getHeaderSearchPaths();
	std::vector<FilePath> storedFrameworkSearchPaths = applicationSettings->getFrameworkSearchPaths();

	applicationSettings->setHeaderSearchPaths({FilePath("test/header/search/path")});
	applicationSettings->setFrameworkSearchPaths({FilePath("test/framework/search/path")});

	generateAndCompareExpectedOutput(
		projectName, std::make_shared<SourceGroupCxxCdb>(sourceGroupSettings));

	applicationSettings->setHeaderSearchPaths(storedHeaderSearchPaths);
	applicationSettings->setFrameworkSearchPaths(storedFrameworkSearchPaths);
}

TEST_CASE("source gropup cxx c correct default standard")
{
	string defaultCStandard = SourceGroupSettingsWithCStandard::getDefaultCStandard();
	string defaultCppStandard = SourceGroupSettingsWithCppStandard::getDefaultCppStandard();

	REQUIRE(  defaultCStandard == "c17");
	REQUIRE(defaultCppStandard == "c++23");
}

#endif	  // BUILD_CXX_LANGUAGE_PACKAGE

#if BUILD_JAVA_LANGUAGE_PACKAGE

TEST_CASE("sourcegroup java empty generates expected output", JAVA_TAG)
{
	const std::string projectName = "java_empty";

	ProjectSettings projectSettings;
	projectSettings.setProjectFilePath("non_existent_project", getInputDirectoryPath(projectName));

	std::shared_ptr<SourceGroupSettingsJavaEmpty> sourceGroupSettings =
		std::make_shared<SourceGroupSettingsJavaEmpty>("fake_id", &projectSettings);
	sourceGroupSettings->setSourceExtensions({".java"});
	sourceGroupSettings->setExcludeFilterStrings({"**/Foo.java"});
	sourceGroupSettings->setJavaStandard({"10"});
	sourceGroupSettings->setSourcePaths({getInputDirectoryPath(projectName).concatenate("src")});
	sourceGroupSettings->setUseJreSystemLibrary(true);
	sourceGroupSettings->setClasspath(
		{getInputDirectoryPath(projectName).concatenate("lib/dependency.jar"),
		 getInputDirectoryPath(projectName).concatenate("classpath_dir")});

	std::shared_ptr<ApplicationSettings> applicationSettings = ApplicationSettings::getInstance();

	std::vector<FilePath> storedJreSystemLibraryPaths =
		applicationSettings->getJreSystemLibraryPaths();

	applicationSettings->setJreSystemLibraryPaths({FilePath("test/jre/system/library/path.jar")});

	generateAndCompareExpectedOutput(
		projectName, std::make_shared<SourceGroupJavaEmpty>(sourceGroupSettings));

	applicationSettings->setJreSystemLibraryPaths(storedJreSystemLibraryPaths);
}

TEST_CASE("sourcegroup java gradle generates expected output", JAVA_TAG)
{
	const std::string projectName = "java_gradle";

	ProjectSettings projectSettings;
	projectSettings.setProjectFilePath("non_existent_project", getInputDirectoryPath(projectName));

	std::shared_ptr<SourceGroupSettingsJavaGradle> sourceGroupSettings =
		std::make_shared<SourceGroupSettingsJavaGradle>("fake_id", &projectSettings);
	sourceGroupSettings->setSourceExtensions({".java"});
	sourceGroupSettings->setExcludeFilterStrings({"**/HelloWorld.java"});
	sourceGroupSettings->setJavaStandard({"10"});
	sourceGroupSettings->setGradleProjectFilePath(
		{getInputDirectoryPath(projectName).concatenate("build.gradle")});
	sourceGroupSettings->setShouldIndexGradleTests(true);

	std::shared_ptr<ApplicationSettings> applicationSettings = ApplicationSettings::getInstance();

	const FilePath storedAppPath = AppPath::getSharedDataDirectoryPath();
	AppPath::setSharedDataDirectoryPath(storedAppPath.getConcatenated("../app").makeAbsolute());

	std::vector<FilePath> storedJreSystemLibraryPaths =
		applicationSettings->getJreSystemLibraryPaths();
	applicationSettings->setJreSystemLibraryPaths({FilePath("test/jre/system/library/path.jar")});

	generateAndCompareExpectedOutput(
		projectName, std::make_shared<SourceGroupJavaGradle>(sourceGroupSettings));

	applicationSettings->setJreSystemLibraryPaths(storedJreSystemLibraryPaths);
	AppPath::setSharedDataDirectoryPath(storedAppPath);
}

TEST_CASE("sourcegroup java maven generates expected output", JAVA_TAG)
{
	std::vector<FilePath> mavenPaths = utility::getMavenExecutablePathDetector()->getPaths();

	REQUIRE(!mavenPaths.empty());

	if (!mavenPaths.empty())
	{
		ApplicationSettings::getInstance()->setMavenPath(mavenPaths.front());
	}

	const std::string projectName = "java_maven";

	ProjectSettings projectSettings;
	projectSettings.setProjectFilePath("non_existent_project", getInputDirectoryPath(projectName));

	std::shared_ptr<SourceGroupSettingsJavaMaven> sourceGroupSettings =
		std::make_shared<SourceGroupSettingsJavaMaven>("fake_id", &projectSettings);
	sourceGroupSettings->setSourceExtensions({".java"});
	sourceGroupSettings->setExcludeFilterStrings({"**/Foo.java"});
	sourceGroupSettings->setJavaStandard({"10"});
	sourceGroupSettings->setMavenProjectFilePath(
		{getInputDirectoryPath(projectName).concatenate("my-app/pom.xml")});
	sourceGroupSettings->setShouldIndexMavenTests(true);

	std::shared_ptr<ApplicationSettings> applicationSettings = ApplicationSettings::getInstance();

	std::vector<FilePath> storedJreSystemLibraryPaths =
		applicationSettings->getJreSystemLibraryPaths();

	applicationSettings->setJreSystemLibraryPaths({FilePath("test/jre/system/library/path.jar")});

	generateAndCompareExpectedOutput(
		projectName, std::make_shared<SourceGroupJavaMaven>(sourceGroupSettings));

	applicationSettings->setJreSystemLibraryPaths(storedJreSystemLibraryPaths);
}

#endif	  // BUILD_JAVA_LANGUAGE_PACKAGE

TEST_CASE("source group custom command generates expected output")
{
	const std::string projectName = "custom_command";

	ProjectSettings projectSettings;
	projectSettings.setProjectFilePath("non_existent_project", getInputDirectoryPath(projectName));

	std::shared_ptr<SourceGroupSettingsCustomCommand> sourceGroupSettings =
		std::make_shared<SourceGroupSettingsCustomCommand>("fake_id", &projectSettings);
	sourceGroupSettings->setCustomCommand("echo \"Hello World\"");
	sourceGroupSettings->setSourcePaths({getInputDirectoryPath(projectName).concatenate("/src")});
	sourceGroupSettings->setSourceExtensions({".txt"});
	sourceGroupSettings->setExcludeFilterStrings({"**/excluded/**"});

	generateAndCompareExpectedOutput(
		projectName, std::make_shared<SourceGroupCustomCommand>(sourceGroupSettings));
}

TEST_CASE("can destroy application instance")
{
	Application::destroyInstance();
	REQUIRE(0 == Application::getInstance().use_count());
}
