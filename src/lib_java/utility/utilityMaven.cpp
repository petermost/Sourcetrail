#include "utilityMaven.h"

#include <cstdlib>

#include "Application.h"
#include "FilePath.h"
#include "MessageStatus.h"
#include "TextAccess.h"
#include "logging.h"
#include "utility.h"
#include "utilityApp.h"
#include "utilityJava.h"
#include "utilityString.h"
#include "utilityXml.h"

using namespace boost::chrono;

namespace
{
void fetchDirectories(
	std::vector<FilePath>& pathList,
	std::shared_ptr<TextAccess> xmlAccess,
	const std::vector<std::string>& tags,
	const FilePath& toAppend = FilePath())
{
	{
		std::string tagString;
		for (size_t i = 0; i < tags.size(); i++)
		{
			if (i != 0)
			{
				tagString += " -> ";
			}
			tagString += tags[i];
		}
		LOG_INFO("Fetching source directories in \"" + tagString + "\".");
	}

	std::vector<std::string> fetchedDirectories = utility::getValuesOfAllXmlElementsOnPath(
		xmlAccess, tags);
	LOG_INFO("Found " + std::to_string(fetchedDirectories.size()) + " source directories.");

	for (const std::string& fetchedDirectory: fetchedDirectories)
	{
		FilePath path(fetchedDirectory);
		if (!toAppend.empty())
		{
			path.concatenate(toAppend);
		}
		pathList.push_back(path);
		LOG_INFO("Found directory \"" + path.str() + "\".");
	}
}

std::string getErrorMessageFromMavenOutput(std::shared_ptr<const TextAccess> mavenOutput)
{
	const std::string errorPrefix = "[ERROR]";
	const std::string fatalPrefix = "[FATAL]";

	std::string errorMessage;

	for (const std::string& line: mavenOutput->getAllLines())
	{
		const std::string trimmedLine = utility::trim(line);

		if (utility::isPrefix(errorPrefix, trimmedLine))
		{
			errorMessage += utility::trim(trimmedLine.substr(errorPrefix.size())) + "\n";
		}
		else if (utility::isPrefix(fatalPrefix, trimmedLine))
		{
			errorMessage += trimmedLine + "\n";
		}
	}

	if (!errorMessage.empty())
	{
		errorMessage = "The following error occurred while executing a Maven command:\n\n" + errorMessage;
	}

	return errorMessage;
}

std::vector<std::string> getMavenArgs(const FilePath& settingsFilePath)
{
	std::vector<std::string> args;
	if (!settingsFilePath.empty() && settingsFilePath.exists())
	{
		args.push_back("--settings \"" + settingsFilePath.str() + "\"");
	}
	return args;
}

}	 // namespace

namespace utility
{
std::string mavenGenerateSources(
	const FilePath& mavenPath, const FilePath& settingsFilePath, const FilePath& projectDirectoryPath)
{
	utility::setJavaHomeVariableIfNotExists();

	auto args = getMavenArgs(settingsFilePath);
	args.push_back("generate-sources");

	std::shared_ptr<TextAccess> outputAccess = TextAccess::createFromString(
		utility::executeProcess(mavenPath.str(), args, projectDirectoryPath, true, milliseconds(60000)).output);

	if (outputAccess->isEmpty())
	{
		return "Sourcetrail was unable to locate Maven on this machine.\n"
			   "Please make sure to provide the correct Maven Path in the preferences.";
	}

	return getErrorMessageFromMavenOutput(outputAccess);
}

bool mavenCopyDependencies(
	const FilePath& mavenPath,
	const FilePath& settingsFilePath,
	const FilePath& projectDirectoryPath,
	const FilePath& outputDirectoryPath)
{
	utility::setJavaHomeVariableIfNotExists();

	auto args = getMavenArgs(settingsFilePath);
	args.push_back("dependency:copy-dependencies");
	args.push_back("-DoutputDirectory=" + outputDirectoryPath.str());

	std::shared_ptr<TextAccess> outputAccess = TextAccess::createFromString(
		utility::executeProcess(mavenPath.str(), args, projectDirectoryPath, true, milliseconds(60000)).output);

	const std::string errorMessage = getErrorMessageFromMavenOutput(outputAccess);
	if (!errorMessage.empty())
	{
		MessageStatus(errorMessage, true, false).dispatch();
		Application::getInstance()->handleDialog(errorMessage);
		return false;
	}

	return !outputAccess->isEmpty();
}

std::vector<FilePath> mavenGetAllDirectoriesFromEffectivePom(
	const FilePath& mavenPath,
	const FilePath& settingsFilePath,
	const FilePath& projectDirectoryPath,
	const FilePath& outputDirectoryPath,
	bool addTestDirectories)
{
	utility::setJavaHomeVariableIfNotExists();

	FilePath outputPath = outputDirectoryPath.getConcatenated(FilePath("/effective-pom.xml"));

	auto args = getMavenArgs(settingsFilePath);
	args.push_back("help:effective-pom");
	args.push_back("-Doutput=" + outputPath.str());

	std::shared_ptr<TextAccess> outputAccess = TextAccess::createFromString(
		utility::executeProcess(mavenPath.str(), args, projectDirectoryPath, true, milliseconds(60000)).output);

	const std::string errorMessage = getErrorMessageFromMavenOutput(outputAccess);
	if (!errorMessage.empty())
	{
		MessageStatus(errorMessage, true, false).dispatch();
		Application::getInstance()->handleDialog(errorMessage);
		return {};
	}
	else if (!outputPath.exists())
	{
		LOG_ERROR("Maven effective-pom didn't generate an output file: " + outputPath.str());
		return {};
	}

	std::shared_ptr<TextAccess> xmlAccess = TextAccess::createFromFile(outputPath);

	std::vector<FilePath> uncheckedDirectories;
	fetchDirectories(
		uncheckedDirectories,
		xmlAccess,
		utility::createVectorFromElements<std::string>("project", "build", "sourceDirectory"));
	fetchDirectories(
		uncheckedDirectories,
		xmlAccess,
		utility::createVectorFromElements<std::string>(
			"projects", "project", "build", "sourceDirectory"));
	fetchDirectories(
		uncheckedDirectories,
		xmlAccess,
		utility::createVectorFromElements<std::string>("project", "build", "directory"),
		FilePath("generated-sources"));
	fetchDirectories(
		uncheckedDirectories,
		xmlAccess,
		utility::createVectorFromElements<std::string>("projects", "project", "build", "directory"),
		FilePath("generated-sources"));

	if (addTestDirectories)
	{
		fetchDirectories(
			uncheckedDirectories,
			xmlAccess,
			utility::createVectorFromElements<std::string>(
				"project", "build", "testSourceDirectory"));
		fetchDirectories(
			uncheckedDirectories,
			xmlAccess,
			utility::createVectorFromElements<std::string>(
				"projects", "project", "build", "testSourceDirectory"));
		fetchDirectories(
			uncheckedDirectories,
			xmlAccess,
			utility::createVectorFromElements<std::string>("project", "build", "directory"),
			FilePath("generated-test-sources"));
		fetchDirectories(
			uncheckedDirectories,
			xmlAccess,
			utility::createVectorFromElements<std::string>(
				"projects", "project", "build", "directory"),
			FilePath("generated-test-sources"));
	}

	std::vector<FilePath> directories;
	for (const FilePath& uncheckedDirectory: uncheckedDirectories)
	{
		if (uncheckedDirectory.exists())
		{
			directories.push_back(uncheckedDirectory);
		}
	}

	LOG_INFO(
		"Found " + std::to_string(directories.size()) + " of " +
		std::to_string(uncheckedDirectories.size()) + " directories on system.");

	return directories;
}
}	 // namespace utility
