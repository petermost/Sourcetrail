#include "Catch2.hpp"

#include "utilitySourceGroupCxx.h"

#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/JSONCompilationDatabase.h>

#include <string>

using namespace std;
using namespace utility;
using namespace clang::tooling;

static const char SOURCETRAIL_WINDOWS_CDB[] = R"(
[
{
  "directory": "D:/Sources/Sourcetrail/build/vcpkg-ninja-release",
  "command": "C:\\PROGRA~1\\MICROS~2\\2022\\COMMUN~1\\VC\\Tools\\MSVC\\1443~1.348\\bin\\HostX64\\x64\\cl.exe  /nologo /TP -DIS_VCPKG_BUILD=1 -DUNICODE -DWIN32_LEAN_AND_MEAN -DWINVER=0x0A00 -D_UNICODE -D_WIN32_WINNT=0x0A00 -ID:\\Sources\\Sourcetrail\\build\\vcpkg-ninja-release\\src\\lib_aidkit -ID:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_aidkit -ID:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_aidkit\\. /DWIN32 /D_WINDOWS /EHsc /O2 /Ob2 /DNDEBUG -std:c++20 -MD /utf-8 /nologo /MP /W4 /permissive- /Zc:__cplusplus /Zc:__STDC__ /Zc:enumTypes /Zc:externConstexpr /Zc:inline /Zc:preprocessor /Zc:templateScope /Zc:throwingNew /w44265 /we4172 /we4840 /wd4245 /wd4250 /wd4389 /wd4456 /wd4457 /wd4458 /Fosrc\\lib_aidkit\\CMakeFiles\\AidKit_lib.dir\\aidkit\\enum_class.cpp.obj /Fdsrc\\lib_aidkit\\CMakeFiles\\AidKit_lib.dir\\AidKit_lib.pdb /FS -c D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_aidkit\\aidkit\\enum_class.cpp",
  "file": "D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_aidkit\\aidkit\\enum_class.cpp",
  "output": "src\\lib_aidkit\\CMakeFiles\\AidKit_lib.dir\\aidkit\\enum_class.cpp.obj"
},
{
  "directory": "D:/Sources/Sourcetrail/build/vcpkg-ninja-release",
  "command": "C:\\PROGRA~1\\MICROS~2\\2022\\COMMUN~1\\VC\\Tools\\MSVC\\1443~1.348\\bin\\HostX64\\x64\\cl.exe  /nologo /TP -DIS_VCPKG_BUILD=1 -DUNICODE -DWIN32_LEAN_AND_MEAN -DWINVER=0x0A00 -D_UNICODE -D_WIN32_WINNT=0x0A00 -ID:\\Sources\\Sourcetrail\\build\\vcpkg-ninja-release\\src\\lib_aidkit -ID:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_aidkit -ID:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_aidkit\\. /DWIN32 /D_WINDOWS /EHsc /O2 /Ob2 /DNDEBUG -std:c++20 -MD /utf-8 /nologo /MP /W4 /permissive- /Zc:__cplusplus /Zc:__STDC__ /Zc:enumTypes /Zc:externConstexpr /Zc:inline /Zc:preprocessor /Zc:templateScope /Zc:throwingNew /w44265 /we4172 /we4840 /wd4245 /wd4250 /wd4389 /wd4456 /wd4457 /wd4458 /Fosrc\\lib_aidkit\\CMakeFiles\\AidKit_lib.dir\\aidkit\\shared_data.cpp.obj /Fdsrc\\lib_aidkit\\CMakeFiles\\AidKit_lib.dir\\AidKit_lib.pdb /FS -c D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_aidkit\\aidkit\\shared_data.cpp",
  "file": "D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_aidkit\\aidkit\\shared_data.cpp",
  "output": "src\\lib_aidkit\\CMakeFiles\\AidKit_lib.dir\\aidkit\\shared_data.cpp.obj"
},
{
  "directory": "D:/Sources/Sourcetrail/build/vcpkg-ninja-release",
  "command": "C:\\PROGRA~1\\MICROS~2\\2022\\COMMUN~1\\VC\\Tools\\MSVC\\1443~1.348\\bin\\HostX64\\x64\\cl.exe  /nologo /TP -DIS_VCPKG_BUILD=1 -ID:\\Sources\\Sourcetrail\\build\\vcpkg-ninja-release\\src\\external -ID:\\Sources\\Sourcetrail\\Sourcetrail\\src\\external -external:ID:\\Sources\\Sourcetrail\\Sourcetrail\\src\\external\\sqlite -external:ID:\\Sources\\Sourcetrail\\build\\vcpkg-ninja-release\\vcpkg_installed\\x64-arm64-linux-windows-osx-static-md\\include -external:W0 /DWIN32 /D_WINDOWS /EHsc /O2 /Ob2 /DNDEBUG -std:c++20 -MD /Fosrc\\external\\CMakeFiles\\External_lib_cppsqlite3.dir\\sqlite\\CppSQLite3.cpp.obj /Fdsrc\\external\\CMakeFiles\\External_lib_cppsqlite3.dir\\External_lib_cppsqlite3.pdb /FS -c D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\external\\sqlite\\CppSQLite3.cpp",
  "file": "D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\external\\sqlite\\CppSQLite3.cpp",
  "output": "src\\external\\CMakeFiles\\External_lib_cppsqlite3.dir\\sqlite\\CppSQLite3.cpp.obj"
},
{
  "directory": "D:/Sources/Sourcetrail/build/vcpkg-ninja-release",
  "command": "C:\\PROGRA~2\\WI3CF2~1\\10\\bin\\100261~1.0\\x64\\rc.exe -DBOOST_CONTAINER_NO_LIB -DBOOST_CONTAINER_STATIC_LINK -DBOOST_DATE_TIME_NO_LIB -DBOOST_DATE_TIME_STATIC_LINK -DBOOST_FILESYSTEM_NO_LIB -DBOOST_FILESYSTEM_STATIC_LINK=1 -DBOOST_LOCALE_NO_LIB -DBOOST_LOCALE_STATIC_LINK -DBOOST_PROGRAM_OPTIONS_NO_LIB -DBOOST_PROGRAM_OPTIONS_STATIC_LINK -DBOOST_UUID_FORCE_AUTO_LINK -DIS_VCPKG_BUILD=1 -DQT_CORE_LIB -DQT_GUI_LIB -DQT_NETWORK_LIB -DQT_NO_DEBUG -DQT_NO_FOREACH -DQT_NO_IMAGEFORMAT_XPM -DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT -DQT_NO_QASCONST -DQT_NO_QSNPRINTF -DQT_SVG_LIB -DQT_USE_NODISCARD_FILE_OPEN -DQT_WIDGETS_LIB -DUNICODE -DU_CHARSET_IS_UTF8=1 -DWIN32 -DWIN32_LEAN_AND_MEAN -DWIN64 -DWINVER=0x0A00 -D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_NONSTDC_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS -D_ENABLE_EXTENDED_ALIGNED_STORAGE -D_SCL_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS -D_UNICODE -D_WIN32_WINNT=0x0A00 -D_WIN64 -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -I D:\\Sources\\Sourcetrail\\build\\vcpkg-ninja-release\\src\\app -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\app -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\app\\paths -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\app -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\component\\controller\\helper -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\component\\controller -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\component\\view\\helper -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\component\\view -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\component -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\data\\bookmark -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\data\\fulltextsearch -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\data\\graph\\token_component -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\data\\graph -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\data\\indexer\\interprocess\\shared_types -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\data\\indexer\\interprocess -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\data\\indexer -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\data\\location -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\data\\name -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\data\\parser -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\data\\search -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\data\\storage\\migration -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\data\\storage\\sqlite -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\data\\storage\\type -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\data\\storage -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\data\\tooltip -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\data -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\project -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\settings\\migration -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\settings\\source_group\\component -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\settings\\source_group\\type -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\settings\\source_group -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\settings -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\utility\\commandline -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\utility\\commandline\\commands -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\utility\\file -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\utility\\interprocess -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\utility\\logging -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\utility\\math -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\utility\\messaging\\filter_types -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\utility\\messaging\\type\\activation -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\utility\\messaging\\type\\bookmark -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\utility\\messaging\\type\\code -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\utility\\messaging\\type\\custom_trail -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\utility\\messaging\\type\\error -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\utility\\messaging\\type\\focus -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\utility\\messaging\\type\\graph -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\utility\\messaging\\type\\history -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\utility\\messaging\\type\\indexing -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\utility\\messaging\\type\\plugin -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\utility\\messaging\\type\\search -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\utility\\messaging\\type\\tab -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\utility\\messaging\\type -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\utility\\messaging -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\utility\\migration -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\utility\\scheduling -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\utility\\text -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\utility -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\settings\\source_group\\component\\cxx -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\settings\\source_group\\component\\java -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib\\. -I D:\\Sources\\Sourcetrail\\build\\vcpkg-ninja-release\\src\\lib -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_aidkit\\. -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_gui\\platform -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_gui\\qt\\element\\bookmark -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_gui\\qt\\element\\button -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_gui\\qt\\element\\code -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_gui\\qt\\element\\history -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_gui\\qt\\element\\search -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_gui\\qt\\element\\dialog -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_gui\\qt\\element\\status -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_gui\\qt\\element -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_gui\\qt\\graphics\\base -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_gui\\qt\\graphics\\component -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_gui\\qt\\graphics\\graph -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_gui\\qt\\graphics -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_gui\\qt\\network -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_gui\\qt\\project_wizard\\content\\path -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_gui\\qt\\project_wizard\\content\\paths -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_gui\\qt\\project_wizard\\content -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_gui\\qt\\project_wizard -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_gui\\qt\\utility -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_gui\\qt\\view -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_gui\\qt\\window -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_gui\\qt -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_gui\\utility\\path_detector -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_gui\\utility -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_gui\\utility\\path_detector\\cxx_header -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_gui\\utility\\path_detector\\java_runtime -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_gui\\utility\\path_detector\\jre_system_library -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_gui\\utility\\path_detector\\maven_executable -I D:\\Sources\\Sourcetrail\\build\\vcpkg-ninja-release\\src\\lib_gui -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\resources\\. -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_cxx\\data\\indexer -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_cxx\\data\\parser\\cxx\\name -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_cxx\\data\\parser\\cxx\\name_resolver -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_cxx\\data\\parser\\cxx -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_cxx\\project -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_cxx\\utility\\codeblocks -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_cxx\\utility -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_cxx\\. -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_java\\data\\indexer -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_java\\data\\parser\\java -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_java\\project -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_java\\utility -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\lib_java\\. -I D:\\Sources\\Sourcetrail\\build\\vcpkg-ninja-release\\vcpkg_installed\\x64-arm64-linux-windows-osx-static-md\\include\\Qt6\\QtWidgets -I D:\\Sources\\Sourcetrail\\build\\vcpkg-ninja-release\\vcpkg_installed\\x64-arm64-linux-windows-osx-static-md\\include\\Qt6 -I D:\\Sources\\Sourcetrail\\build\\vcpkg-ninja-release\\vcpkg_installed\\x64-arm64-linux-windows-osx-static-md\\include\\Qt6\\QtCore -I D:\\Sources\\Sourcetrail\\build\\vcpkg-ninja-release\\vcpkg_installed\\x64-arm64-linux-windows-osx-static-md\\share\\Qt6\\mkspecs\\win32-msvc -I D:\\Sources\\Sourcetrail\\build\\vcpkg-ninja-release\\vcpkg_installed\\x64-arm64-linux-windows-osx-static-md\\include\\Qt6\\QtGui -I D:\\Sources\\Sourcetrail\\build\\vcpkg-ninja-release\\vcpkg_installed\\x64-arm64-linux-windows-osx-static-md\\include\\Qt6\\QtNetwork -I D:\\Sources\\Sourcetrail\\build\\vcpkg-ninja-release\\vcpkg_installed\\x64-arm64-linux-windows-osx-static-md\\include\\Qt6\\QtSvg -I D:\\Sources\\Sourcetrail\\build\\vcpkg-ninja-release\\vcpkg_installed\\x64-arm64-linux-windows-osx-static-md\\include -I \"C:\\Program Files\\OpenJDK\\jdk-22.0.2\\include\" -I \"C:\\Program Files\\OpenJDK\\jdk-22.0.2\\include\\win32\" -I D:\\Sources\\Sourcetrail\\Sourcetrail\\src\\external\\sqlite -DWIN32 /fo src\\app\\CMakeFiles\\Sourcetrail.dir\\Sourcetrail.rc.res D:\\Sources\\Sourcetrail\\build\\vcpkg-ninja-release\\src\\app\\Sourcetrail.rc",
  "file": "D:\\Sources\\Sourcetrail\\build\\vcpkg-ninja-release\\src\\app\\Sourcetrail.rc",
  "output": "src\\app\\CMakeFiles\\Sourcetrail.dir\\Sourcetrail.rc.res"
}
])";

static const char SOURCETRAIL_LINUX_CDB[] = R"(
[
{
  "directory": "/home/peter/Sources/Sourcetrail/build/vcpkg-ninja-release",
  "command": "/usr/bin/c++ -DIS_VCPKG_BUILD=1 -I/home/peter/Sources/Sourcetrail/build/vcpkg-ninja-release/src/lib_aidkit -I/home/peter/Sources/Sourcetrail/Sourcetrail/src/lib_aidkit -I/home/peter/Sources/Sourcetrail/Sourcetrail/src/lib_aidkit/. -O3 -DNDEBUG -std=c++20 -fdiagnostics-color=always -finput-charset=UTF-8 -pipe -Wall -Wextra -Wpedantic -Wcast-align=strict -Wcast-qual -Wduplicated-branches -Wduplicated-cond -Wextra-semi -Wformat-overflow=2 -Wformat-truncation=2 -Wlogical-op -Wnon-virtual-dtor -Wuninitialized -Wunused -Winit-self -Wwrite-strings -Werror=narrowing -Werror=suggest-override -Wno-comment -Wno-implicit-fallthrough -Wno-missing-field-initializers -Wno-stringop-truncation -Wno-unknown-pragmas -o src/lib_aidkit/CMakeFiles/AidKit_lib.dir/aidkit/enum_class.cpp.o -c /home/peter/Sources/Sourcetrail/Sourcetrail/src/lib_aidkit/aidkit/enum_class.cpp",
  "file": "/home/peter/Sources/Sourcetrail/Sourcetrail/src/lib_aidkit/aidkit/enum_class.cpp",
  "output": "src/lib_aidkit/CMakeFiles/AidKit_lib.dir/aidkit/enum_class.cpp.o"
},
{
  "directory": "/home/peter/Sources/Sourcetrail/build/vcpkg-ninja-release",
  "command": "/usr/bin/c++ -DIS_VCPKG_BUILD=1 -I/home/peter/Sources/Sourcetrail/build/vcpkg-ninja-release/src/lib_aidkit -I/home/peter/Sources/Sourcetrail/Sourcetrail/src/lib_aidkit -I/home/peter/Sources/Sourcetrail/Sourcetrail/src/lib_aidkit/. -O3 -DNDEBUG -std=c++20 -fdiagnostics-color=always -finput-charset=UTF-8 -pipe -Wall -Wextra -Wpedantic -Wcast-align=strict -Wcast-qual -Wduplicated-branches -Wduplicated-cond -Wextra-semi -Wformat-overflow=2 -Wformat-truncation=2 -Wlogical-op -Wnon-virtual-dtor -Wuninitialized -Wunused -Winit-self -Wwrite-strings -Werror=narrowing -Werror=suggest-override -Wno-comment -Wno-implicit-fallthrough -Wno-missing-field-initializers -Wno-stringop-truncation -Wno-unknown-pragmas -o src/lib_aidkit/CMakeFiles/AidKit_lib.dir/aidkit/shared_data.cpp.o -c /home/peter/Sources/Sourcetrail/Sourcetrail/src/lib_aidkit/aidkit/shared_data.cpp",
  "file": "/home/peter/Sources/Sourcetrail/Sourcetrail/src/lib_aidkit/aidkit/shared_data.cpp",
  "output": "src/lib_aidkit/CMakeFiles/AidKit_lib.dir/aidkit/shared_data.cpp.o"
},
{
  "directory": "/home/peter/Sources/Sourcetrail/build/vcpkg-ninja-release",
  "command": "/usr/bin/c++ -DIS_VCPKG_BUILD=1 -I/home/peter/Sources/Sourcetrail/build/vcpkg-ninja-release/src/external -I/home/peter/Sources/Sourcetrail/Sourcetrail/src/external -isystem /home/peter/Sources/Sourcetrail/Sourcetrail/src/external/sqlite -isystem /home/peter/Sources/Sourcetrail/build/vcpkg-ninja-release/vcpkg_installed/x64-arm64-linux-windows-osx-static-md/include -O3 -DNDEBUG -std=c++20 -fdiagnostics-color=always -o src/external/CMakeFiles/External_lib_cppsqlite3.dir/sqlite/CppSQLite3.cpp.o -c /home/peter/Sources/Sourcetrail/Sourcetrail/src/external/sqlite/CppSQLite3.cpp",
  "file": "/home/peter/Sources/Sourcetrail/Sourcetrail/src/external/sqlite/CppSQLite3.cpp",
  "output": "src/external/CMakeFiles/External_lib_cppsqlite3.dir/sqlite/CppSQLite3.cpp.o"
}
])";

static const char ISSUE_WINDOWS_CDB[] = R"(
[
{
  "directory": "D:/code/reflector/cmake-build-release",
  "command": "C:\\PROGRA~1\\MIB055~1\\2022\\COMMUN~1\\VC\\Tools\\MSVC\\1443~1.348\\bin\\Hostx86\\x64\\cl.exe  /nologo /TP -DQT_CONCURRENT_LIB -DQT_CORE_LIB -DQT_GUI_LIB -DQT_NETWORK_LIB -DQT_NO_DEBUG -DQT_OPENGL_LIB -DQT_QMLINTEGRATION_LIB -DQT_QMLMODELS_LIB -DQT_QML_LIB -DQT_QUICK_LIB -DUNICODE -DWIN32 -DWIN64 -D_ENABLE_EXTENDED_ALIGNED_STORAGE -D_UNICODE -D_WIN64 -ID:\\code\\reflector\\cmake-build-release\\appreflector_autogen\\include -ID:\\code\\reflector -ID:\\code\\reflector\\src -external:IC:\\Qt\\6.5.3\\msvc2019_64\\include\\QtQml\\6.5.3 -external:IC:\\Qt\\6.5.3\\msvc2019_64\\include\\QtQml\\6.5.3\\QtQml -external:IC:\\Qt\\6.5.3\\msvc2019_64\\include\\QtCore\\6.5.3 -external:IC:\\Qt\\6.5.3\\msvc2019_64\\include\\QtCore\\6.5.3\\QtCore -external:IC:\\Qt\\6.5.3\\msvc2019_64\\include\\QtCore -external:IC:\\Qt\\6.5.3\\msvc2019_64\\include -external:IC:\\Qt\\6.5.3\\msvc2019_64\\mkspecs\\win32-msvc -external:IC:\\Qt\\6.5.3\\msvc2019_64\\include\\QtQml -external:IC:\\Qt\\6.5.3\\msvc2019_64\\include\\QtQmlIntegration -external:IC:\\Qt\\6.5.3\\msvc2019_64\\include\\QtNetwork -external:IC:\\Qt\\6.5.3\\msvc2019_64\\include\\QtQuick -external:IC:\\Qt\\6.5.3\\msvc2019_64\\include\\QtGui -external:IC:\\Qt\\6.5.3\\msvc2019_64\\include\\QtQmlModels -external:IC:\\Qt\\6.5.3\\msvc2019_64\\include\\QtOpenGL -external:IC:\\Qt\\6.5.3\\msvc2019_64\\include\\QtConcurrent -external:W0 /DWIN32 /D_WINDOWS /GR /EHsc /O2 /Ob2 /DNDEBUG -std:c++17 -MD -Zc:__cplusplus -permissive- -utf-8 /FoCMakeFiles\\appreflector.dir\\src\\TerrainItem.cpp.obj /FdCMakeFiles\\appreflector.dir\\ /FS -c D:\\code\\reflector\\src\\TerrainItem.cpp",
  "file": "D:\\code\\reflector\\src\\TerrainItem.cpp",
  "output": "CMakeFiles\\appreflector.dir\\src\\TerrainItem.cpp.obj"
}
])";

static vector<CompileCommand> loadDatabase(string_view cdbContent, JSONCommandLineSyntax syntax)
{
	string error;
	shared_ptr<JSONCompilationDatabase> cdb = loadCDB(cdbContent, syntax, &error);
	REQUIRE((cdb != nullptr && error.empty()));

	return cdb->getAllCompileCommands();
}

TEST_CASE("CDB replace msvc arguments in windows database")
{
	vector<string> commandLine;

	vector<CompileCommand> compileCommands = loadDatabase(SOURCETRAIL_WINDOWS_CDB, JSONCommandLineSyntax::Windows);
	REQUIRE(compileCommands.size() == 4);

	// aidkit command lines:

	commandLine = compileCommands[0].CommandLine;
	REQUIRE(commandLine.size() == 47);
	replaceMsvcArguments(&commandLine);
	REQUIRE(commandLine.size() == 17);

	commandLine = compileCommands[1].CommandLine;
	REQUIRE(commandLine.size() == 47);
	replaceMsvcArguments(&commandLine);
	REQUIRE(commandLine.size() == 17);

	// CppSQLite3 command line:

	commandLine = compileCommands[2].CommandLine;
	REQUIRE(commandLine.size() == 22);
	replaceMsvcArguments(&commandLine);
	REQUIRE(commandLine.size() == 14);

	// Resource compiler (Sourcetrail.rc):

	commandLine = compileCommands[3].CommandLine;
	REQUIRE(commandLine.size() == 280);
	replaceMsvcArguments(&commandLine);
	REQUIRE(commandLine.size() == 280);
}

TEST_CASE("CDB replace msvc arguments in linux database")
{
	vector<string> commandLine;

	vector<CompileCommand> compileCommands = loadDatabase(SOURCETRAIL_LINUX_CDB, JSONCommandLineSyntax::Gnu);
	REQUIRE(compileCommands.size() == 3);

	// aidkit command lines:

	commandLine = compileCommands[0].CommandLine;
	REQUIRE(commandLine.size() == 38);
	replaceMsvcArguments(&commandLine);
	REQUIRE(commandLine.size() == 38);

	commandLine = compileCommands[1].CommandLine;
	REQUIRE(commandLine.size() == 38);
	replaceMsvcArguments(&commandLine);
	REQUIRE(commandLine.size() == 38);

	// CppSQLite3 command line:

	commandLine = compileCommands[2].CommandLine;
	REQUIRE(commandLine.size() == 16);
	replaceMsvcArguments(&commandLine);
	REQUIRE(commandLine.size() == 16);
}

TEST_CASE("CDB replace msvc arguments in issue database")
{
	vector<CompileCommand> compileCommands = loadDatabase(ISSUE_WINDOWS_CDB, JSONCommandLineSyntax::Windows);
	REQUIRE(compileCommands.size() == 1);

	vector<string> commandLine = compileCommands[0].CommandLine;
	REQUIRE(commandLine.size() == 55);
	replaceMsvcArguments(&commandLine);
	REQUIRE(commandLine.size() == 46);
}

TEST_CASE("CDB replace msvc arguments")
{
	vector<string> arguments {
		"C:\\some_path\\cl.exe",
		"/DDefine",
		"/UUndefine",

		"/IIncludeDirectory",
		"/external:IExternalIncludeDirectory1", "-external:IExternalIncludeDirectory2",
		"/FIIncludeFile1", 	"-FIIncludeFile2",

		"/std:c++latest", "-std:c++latest",
		"/std:c++11", "-std:c++14",
		"/std:clatest", "-std:clatest",
		"/std:c11", "-std:c99",

		"/SomeUnknownOption"
	};

	const vector<string> expectedClangArguments {
		"C:\\some_path\\cl.exe",
		"-DDefine",
		"-UUndefine",

		"-IIncludeDirectory",
		"-isystem ExternalIncludeDirectory1", "-isystem ExternalIncludeDirectory2",
		"-include IncludeFile1", "-include IncludeFile2",

		"-std=c++23", "-std=c++23",
		"-std=c++11", "-std=c++14",
		"-std=c17", "-std=c17",
		"-std=c11", "-std=c99",

		// Removed: "/SomeUnknownOption"
	};
	replaceMsvcArguments(&arguments);

	for (size_t i = 0; i < arguments.size() && i < expectedClangArguments.size(); ++i)
		REQUIRE(arguments[i] == expectedClangArguments[i]);
}

