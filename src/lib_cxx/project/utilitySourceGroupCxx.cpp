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

shared_ptr<CompilationDatabase> loadCDB(const FilePath& cdbPath, std::string* error)
{
	unique_ptr<CompilationDatabase> cdb;
	
	if (cdbPath.empty() || !cdbPath.exists())
		return cdb;

	string errorString;
	cdb = JSONCompilationDatabase::loadFromFile(cdbPath.str(), errorString, JSONCommandLineSyntax::AutoDetect);
	if (cdb == nullptr && error != nullptr)
		*error = errorString;

	if (cdb != nullptr)
		cdb = expandResponseFiles(std::move(cdb), llvm::vfs::getRealFileSystem());

	return cdb;
}

shared_ptr<CompilationDatabase> loadCDB(string_view cdbContent, JSONCommandLineSyntax syntax, string *error)
{
	unique_ptr<CompilationDatabase> cdb;

	if (cdbContent.empty())
		return cdb;
		
	string errorString;
	cdb = JSONCompilationDatabase::loadFromBuffer(cdbContent, errorString, syntax);
	if (cdb == nullptr && error != nullptr)
		*error = errorString;

	return cdb;
}

bool containsIncludePchFlags(std::shared_ptr<clang::tooling::CompilationDatabase> cdb)
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

static optional<string> getArgumentValue(const string &argument, string_view argumentKey)
{
	return argument.starts_with(argumentKey) ? make_optional(argument.substr(argumentKey.length())) : nullopt;
}

void replaceMsvcArguments(vector<string> *commandLineArguments)
{
	// Replace/Remove arguments only if these are for the Microsoft compiler, otherwise the check for '/' will remove Linux paths:

	if (commandLineArguments->size() >= 1 && !(*commandLineArguments)[0].ends_with("cl.exe"s))
		return;

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
			*argument++ = "-D"s + *argumentValue;
		else if ((argumentValue = getArgumentValue(*argument, "/U"sv)))
			*argument++ = "-U"s + *argumentValue;
		else if ((argumentValue = getArgumentValue(*argument, "/FI"sv)) || (argumentValue = getArgumentValue(*argument, "-FI"sv)))
			*argument++ = "-include "s + *argumentValue;

		// Preprocessor include directories:

		else if ((argumentValue = getArgumentValue(*argument, "/I"sv)))
			*argument++ = "-I"s + *argumentValue;
		else if ((argumentValue = getArgumentValue(*argument, "/external:I"sv)) || (argumentValue = getArgumentValue(*argument, "-external:I"sv)))
			*argument++ = "-isystem "s + *argumentValue;

		// C/C++ language version selection (no support for previews):

		else if (argument->starts_with("/std:c++latest"sv) || argument->starts_with("-std:c++latest"sv))
			*argument++ = "-std="s + ClangVersionSupport::getLatestCppStandard();
		else if (argument->starts_with("/std:clatest"sv) || argument->starts_with("-std:clatest"sv))
			*argument++ = "-std="s + ClangVersionSupport::getLatestCStandard();
		else if ((argumentValue = getArgumentValue(*argument, "/std:c++"sv)) || (argumentValue = getArgumentValue(*argument, "-std:c++"sv)))
			*argument++ = "-std=c++"s + *argumentValue;
		else if ((argumentValue = getArgumentValue(*argument, "/std:c"sv)) || (argumentValue = getArgumentValue(*argument, "-std:c"sv)))
			*argument++ = "-std=c"s + *argumentValue;
		else if (argument->starts_with('/'))
			argument = commandLineArguments->erase(argument);
		else
			++argument;
	}
}

}	 // namespace utility
