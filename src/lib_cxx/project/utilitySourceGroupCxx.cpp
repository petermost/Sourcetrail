#include "utilitySourceGroupCxx.h"

#include <clang/Tooling/JSONCompilationDatabase.h>

#include "CanonicalFilePathCache.h"
#include "CxxCompilationDatabaseSingle.h"
#include "CxxDiagnosticConsumer.h"
#include "CxxParser.h"
#include "DialogView.h"
#include "FilePathFilter.h"
#include "FileRegister.h"
#include "FileSystem.h"
#include "GeneratePCHAction.h"
#include "ParserClientImpl.h"
#include "SingleFrontendActionFactory.h"
#include "SourceGroupSettingsWithCxxPchOptions.h"
#include "StorageProvider.h"
#include "TaskLambda.h"
#include "logging.h"
#include "utility.h"
#include "utilityString.h"
#include "ToolVersionSupport.h"

using namespace std;
using namespace clang::tooling;

namespace utility
{
std::shared_ptr<Task> createBuildPchTask(const SourceGroupSettingsWithCxxPchOptions *settings, std::vector<std::string> compilerFlags,
	std::shared_ptr<StorageProvider> storageProvider, std::shared_ptr<DialogView> dialogView)
{
	shared_ptr<TaskLambda> pchTask(make_shared<TaskLambda>([](){}));
	
	FilePath pchInputFilePath = settings->getPchInputFilePathExpandedAndAbsolute();
	FilePath pchDependenciesDirectoryPath = settings->getPchDependenciesDirectoryPath();

	if (pchInputFilePath.empty() || pchDependenciesDirectoryPath.empty())
	{
		return pchTask;
	}

	if (!pchInputFilePath.exists())
	{
		LOG_ERROR("Precompiled header input file \"" + pchInputFilePath.str() + "\" does not exist.");
		return pchTask;
	}

	const FilePath pchOutputFilePath = pchDependenciesDirectoryPath.getConcatenated(pchInputFilePath.fileName()).replaceExtension("pch");

	utility::removeIncludePchFlag(compilerFlags);
	compilerFlags.push_back(pchInputFilePath.str());
	compilerFlags.push_back("-emit-pch");
	compilerFlags.push_back("-o");
	compilerFlags.push_back(pchOutputFilePath.str());

	pchTask = std::make_shared<TaskLambda>([dialogView, storageProvider, pchInputFilePath, pchOutputFilePath, compilerFlags]()
	{
		dialogView->showUnknownProgressDialog("Preparing Indexing", "Processing Precompiled Headers");
		LOG_INFO("Generating precompiled header output for input file \"" + pchInputFilePath.str() + "\" at location \"" + pchOutputFilePath.str() + "\"");
		
		CxxParser::initializeLLVM();
		
		if (!pchOutputFilePath.getParentDirectory().exists())
		{
			FileSystem::createDirectories(pchOutputFilePath.getParentDirectory());
		}
		
		std::shared_ptr<IntermediateStorage> storage = std::make_shared<IntermediateStorage>();
		std::shared_ptr<ParserClientImpl> client = std::make_shared<ParserClientImpl>(storage);
		
		std::shared_ptr<FileRegister> fileRegister = std::make_shared<FileRegister>(pchInputFilePath, std::set<FilePath>{pchInputFilePath}, std::set<FilePathFilter>{});
		
		std::shared_ptr<CanonicalFilePathCache> canonicalFilePathCache = std::make_shared<CanonicalFilePathCache>(fileRegister);
		
		clang::tooling::CompileCommand pchCommand;
		pchCommand.Filename = pchInputFilePath.fileName();
		pchCommand.Directory = pchOutputFilePath.getParentDirectory().str();
		// DON'T use "-fsyntax-only" here because it will cause the output file to be erased
		pchCommand.CommandLine = utility::concat({"clang-tool"}, CxxParser::getCommandlineArgumentsEssential(compilerFlags));
		
		CxxCompilationDatabaseSingle compilationDatabase(pchCommand);
		clang::tooling::ClangTool tool(compilationDatabase, {pchInputFilePath.str()});
		GeneratePCHAction *action = new GeneratePCHAction(client, canonicalFilePathCache); // TODO (petermost): Memory leak?
		
		clang::DiagnosticOptions *options = new clang::DiagnosticOptions(); // TODO (petermost): Memory leak?
		CxxDiagnosticConsumer diagnostics(llvm::errs(), options, client, canonicalFilePathCache, pchInputFilePath, true);
		
		tool.setDiagnosticConsumer(&diagnostics);
		tool.clearArgumentsAdjusters();
		tool.run(new SingleFrontendActionFactory(action)); // TODO (petermost): Memory leak?
		
		storageProvider->insert(storage);
	});
	return pchTask;
}

shared_ptr<JSONCompilationDatabase> loadCDB(const FilePath& cdbPath, std::string* error)
{
	shared_ptr<JSONCompilationDatabase> cdb;
	
	if (cdbPath.empty() || !cdbPath.exists())
		return cdb;

	string errorString;
	cdb = JSONCompilationDatabase::loadFromFile(cdbPath.str(), errorString, JSONCommandLineSyntax::AutoDetect);

	if (error != nullptr && !errorString.empty())
		*error = errorString;

	return cdb;
}

shared_ptr<JSONCompilationDatabase> loadCDB(string_view cdbContent, JSONCommandLineSyntax syntax, string *error)
{
	shared_ptr<JSONCompilationDatabase> cdb;

	if (cdbContent.empty())
		return cdb;
		
	string errorString;
	cdb = JSONCompilationDatabase::loadFromBuffer(cdbContent, errorString, syntax);

	if (error != nullptr && !errorString.empty())
		*error = errorString;

	return cdb;
}

bool containsIncludePchFlags(std::shared_ptr<clang::tooling::JSONCompilationDatabase> cdb)
{
	for (const clang::tooling::CompileCommand& command: cdb->getAllCompileCommands())
	{
		if (containsIncludePchFlag(command.CommandLine))
		{
			return true;
		}
	}
	return false;
}

bool containsIncludePchFlag(const std::vector<std::string>& args)
{
	const std::string includePchPrefix = "-include-pch";
	for (size_t i = 0; i < args.size(); i++)
	{
		const std::string arg = utility::trim(args[i]);
		if (utility::isPrefix(includePchPrefix, arg))
		{
			return true;
		}
	}
	return false;
}

std::vector<std::string> getWithRemoveIncludePchFlag(const std::vector<std::string>& args)
{
	std::vector<std::string> ret = args;
	removeIncludePchFlag(ret);
	return ret;
}

void removeIncludePchFlag(std::vector<std::string>& args)
{
	const std::string includePchPrefix = "-include-pch";
	for (size_t i = 0; i < args.size(); i++)
	{
		const std::string arg = utility::trim(args[i]);
		if (utility::isPrefix(includePchPrefix, arg))
		{
			if (i + 1 < args.size() &&
				!utility::isPrefix("-", utility::trim(args[i + 1])) &&
				arg == includePchPrefix)
			{
				args.erase(args.begin() + i + 1);
			}
			args.erase(args.begin() + i);
			i--;
		}
	}
}

std::vector<std::string> getIncludePchFlags(const SourceGroupSettingsWithCxxPchOptions* settings)
{
	const FilePath pchInputFilePath = settings->getPchInputFilePathExpandedAndAbsolute();
	const FilePath pchDependenciesDirectoryPath = settings->getPchDependenciesDirectoryPath();

	if (!pchInputFilePath.empty() && !pchDependenciesDirectoryPath.empty())
	{
		const FilePath pchOutputFilePath = pchDependenciesDirectoryPath
											   .getConcatenated(pchInputFilePath.fileName())
											   .replaceExtension("pch");

		return {"-fallow-pch-with-compiler-errors", "-include-pch", pchOutputFilePath.str()};
	}

	return {};
}

static bool getArgumentValue(const string &argument, string_view argumentKey, string *argumentValue)
{
	if (argument.starts_with(argumentKey))
	{
		*argumentValue = argument.substr(argumentKey.length());
		return true;
	}
	else
		return false;
}

void replaceMsvcArguments(vector<string> *arguments)
{
	// Replace/Remove arguments only if these are for the Microsoft compiler, otherwise the check for '/' will remove Linux paths:

	if (arguments->size() >= 1 && !(*arguments)[0].ends_with("cl.exe"s))
		return;

	string clangArgument;
	string argumentValue;
	vector<string> clangArguments;

	// Keep/Replace only those options which are necessary to parse the code correctly:

	for (const string &argument : *arguments)
	{
		// Documentation excerpt (https://learn.microsoft.com/en-us/cpp/build/reference/compiler-options):
		// - All compiler options are case-sensitive.
		// - You may use either a forward slash (/) or a dash (-) to specify a compiler option.

		clangArgument.clear();

		// Preprocessor symbols:

		if (getArgumentValue(argument, "/D"sv, &argumentValue))
			clangArgument = "-D"s + argumentValue;
		else if (getArgumentValue(argument, "/U"sv, &argumentValue))
			clangArgument = "-U"s + argumentValue;
		else if (getArgumentValue(argument, "/FI"sv, &argumentValue) || getArgumentValue(argument, "-FI"sv, &argumentValue))
			clangArgument = "-include "s + argumentValue;

		// Preprocessor include directories:

		else if (getArgumentValue(argument, "/I"sv, &argumentValue))
			clangArgument = "-I"s + argumentValue;
		else if (getArgumentValue(argument, "/external:I"sv, &argumentValue) || getArgumentValue(argument, "-external:I"sv, &argumentValue))
			clangArgument = "-isystem "s + argumentValue;

		// C/C++ language version selection (no support for previews):

		else if (argument.starts_with("/std:c++latest"sv) || argument.starts_with("-std:c++latest"sv))
			clangArgument = "-std="s + ClangVersionSupport::getLatestCppStandard();
		else if (argument.starts_with("/std:clatest"sv) || argument.starts_with("-std:clatest"sv))
			clangArgument = "-std="s + ClangVersionSupport::getLatestCStandard();
		else if (getArgumentValue(argument, "/std:c++"sv, &argumentValue) || getArgumentValue(argument, "-std:c++"sv, &argumentValue))
			clangArgument = "-std=c++"s + argumentValue;
		else if (getArgumentValue(argument, "/std:c"sv, &argumentValue) || getArgumentValue(argument, "-std:c"sv, &argumentValue))
			clangArgument = "-std=c"s + argumentValue;

		if (!clangArgument.empty())
			clangArguments.push_back(clangArgument);
		else if (!argument.starts_with('/'))
			clangArguments.push_back(argument);
	}
	*arguments = std::move(clangArguments);
}

}	 // namespace utility
