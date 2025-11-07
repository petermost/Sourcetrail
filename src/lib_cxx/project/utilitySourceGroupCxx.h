#ifndef UTILITY_SOURCE_GROUP_CXX_H
#define UTILITY_SOURCE_GROUP_CXX_H

#include <memory>
#include <string>
#include <vector>

#include <clang/Tooling/JSONCompilationDatabase.h>

class DialogView;
class FilePath;
class SourceGroupSettingsWithCxxPchOptions;
class StorageProvider;
class Task;

namespace utility
{
std::shared_ptr<Task> createBuildPchTask(
	const SourceGroupSettingsWithCxxPchOptions* settings,
	std::vector<std::string> compilerFlags,
	std::shared_ptr<StorageProvider> storageProvider,
	std::shared_ptr<DialogView> dialogView);

std::shared_ptr<clang::tooling::CompilationDatabase> loadCDB(const FilePath& cdbPath, std::string* error = nullptr);
	
std::shared_ptr<clang::tooling::CompilationDatabase> loadCDB(std::string_view cdbContent, clang::tooling::JSONCommandLineSyntax syntax,
	std::string *error);

bool containsIncludePchFlags(std::shared_ptr<clang::tooling::CompilationDatabase> cdb);
bool containsIncludePchFlag(const std::vector<std::string>& args);
std::vector<std::string> getWithRemoveIncludePchFlag(const std::vector<std::string>& args);
void removeIncludePchFlag(std::vector<std::string>& args);
std::vector<std::string> getIncludePchFlags(const SourceGroupSettingsWithCxxPchOptions* settings);
void replaceMsvcArguments(std::vector<std::string> *args);
}

#endif
