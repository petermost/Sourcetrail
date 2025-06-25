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

// Excerpt (https://learn.microsoft.com/en-us/cpp/build/reference/compiler-options):
// - All compiler options are case-sensitive.
// - You may use either a forward slash (/) or a dash (-) to specify a compiler option.

static constexpr string_view DEFINE_SWITCH("/D");
static constexpr string_view UNDEFINE_SWITCH("/U");

static constexpr string_view INCLUDE_SWITCH("/I");
static constexpr string_view EXTERNAL_INCLUDE_SWITCH_1("/external:I");
static constexpr string_view EXTERNAL_INCLUDE_SWITCH_2("-external:I");

static constexpr string_view STD_VERSION_SWITCH_1("/std:");
static constexpr string_view STD_VERSION_SWITCH_2("-std:");

static constexpr string_view FORCE_INCLUDE_SWITCH_1("/FI");
static constexpr string_view FORCE_INCLUDE_SWITCH_2("-FI");

void replaceMsvcFlags(vector<string> *args)
{
	// Replace/Remove flags only if these arguments are for the Microsoft compiler:
	if (args->size() >= 1 && !(*args)[0].ends_with("cl.exe"s))
		return;

	string clangArg;
	vector<string> clangArgs;
	
   // Keep/Replace only those options which are necessary to parse the code correctly:

	for (const string &arg : *args)
	{
		clangArg.clear();

		// Defines/Undefines:

		if (arg.starts_with(DEFINE_SWITCH))
			clangArg = "-D"s + arg.substr(DEFINE_SWITCH.length());
		else if (arg.starts_with(UNDEFINE_SWITCH))
			clangArg = "-U"s + arg.substr(UNDEFINE_SWITCH.length());
		else if (arg.starts_with(FORCE_INCLUDE_SWITCH_1))
			clangArg = "-include "s + arg.substr(FORCE_INCLUDE_SWITCH_1.length());
		else if (arg.starts_with(FORCE_INCLUDE_SWITCH_2))
			clangArg = "-include "s + arg.substr(FORCE_INCLUDE_SWITCH_2.length());

		// Include directories:

		else if (arg.starts_with(INCLUDE_SWITCH))
			clangArg = "-I"s + arg.substr(INCLUDE_SWITCH.length());
		else if (arg.starts_with(EXTERNAL_INCLUDE_SWITCH_1))
			clangArg = "-isystem "s + arg.substr(EXTERNAL_INCLUDE_SWITCH_1.length());
		else if (arg.starts_with(EXTERNAL_INCLUDE_SWITCH_2))
			clangArg = "-isystem "s + arg.substr(EXTERNAL_INCLUDE_SWITCH_2.length());

		// First the specific std switches:

		else if (arg.starts_with("/std:c++latest"s))
			clangArg = "-std="s + ClangVersionSupport::getLatestCppStandard();
		else if (arg.starts_with("/std:c++23preview"s))
			clangArg = "-std="s + ClangVersionSupport::getLatestCppDraft();

		// Then the general std switches:

		else if (arg.starts_with(STD_VERSION_SWITCH_1))
			clangArg = "-std="s + arg.substr(STD_VERSION_SWITCH_1.length());
		else if (arg.starts_with(STD_VERSION_SWITCH_2))
			clangArg = "-std="s + arg.substr(STD_VERSION_SWITCH_2.length());

		if (!clangArg.empty())
			clangArgs.push_back(clangArg);
		else if (!arg.starts_with('/'))
			clangArgs.push_back(arg);
	}
	*args = clangArgs;
}

}	 // namespace utility
