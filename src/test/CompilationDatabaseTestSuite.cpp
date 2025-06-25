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

TEST_CASE("CDB replace msvc flags in windows database")
{
	vector<string> commandLine;

	vector<CompileCommand> compileCommands = loadDatabase(SOURCETRAIL_WINDOWS_CDB, JSONCommandLineSyntax::Windows);
	REQUIRE(compileCommands.size() == 3);

	// aidkit command lines:

	commandLine = compileCommands[0].CommandLine;
	REQUIRE(commandLine.size() == 47);
	replaceMsvcFlags(&commandLine);
	REQUIRE(commandLine.size() == 17);

	commandLine = compileCommands[1].CommandLine;
	REQUIRE(commandLine.size() == 47);
	replaceMsvcFlags(&commandLine);
	REQUIRE(commandLine.size() == 17);

	// CppSQLite3 command line:

	commandLine = compileCommands[2].CommandLine;
	REQUIRE(commandLine.size() == 22);
	replaceMsvcFlags(&commandLine);
	REQUIRE(commandLine.size() == 14);
}

TEST_CASE("CDB replace msvc flags in linux database")
{
	vector<string> commandLine;

	vector<CompileCommand> compileCommands = loadDatabase(SOURCETRAIL_LINUX_CDB, JSONCommandLineSyntax::Gnu);
	REQUIRE(compileCommands.size() == 3);

	// aidkit command lines:

	commandLine = compileCommands[0].CommandLine;
	REQUIRE(commandLine.size() == 38);
	replaceMsvcFlags(&commandLine);
	REQUIRE(commandLine.size() == 38);

	commandLine = compileCommands[1].CommandLine;
	REQUIRE(commandLine.size() == 38);
	replaceMsvcFlags(&commandLine);
	REQUIRE(commandLine.size() == 38);

	// CppSQLite3 command line:

	commandLine = compileCommands[2].CommandLine;
	REQUIRE(commandLine.size() == 16);
	replaceMsvcFlags(&commandLine);
	REQUIRE(commandLine.size() == 16);
}

TEST_CASE("CDB replace msvc flags in issue database")
{
	vector<CompileCommand> compileCommands = loadDatabase(ISSUE_WINDOWS_CDB, JSONCommandLineSyntax::Windows);
	REQUIRE(compileCommands.size() == 1);

	vector<string> commandLine = compileCommands[0].CommandLine;
	REQUIRE(commandLine.size() == 55);
	replaceMsvcFlags(&commandLine);
	REQUIRE(commandLine.size() == 46);
}

TEST_CASE("CDB replace msvc flags")
{
	vector<string> args(13);
	args[0]  = "C:\\some_path\\cl.exe";
	args[1]  = "/DDefine";
	args[2]  = "/UUndefine";
	args[3]  = "/IIncludeDirectory";
	args[4]  = "/external:IExternalIncludeDirectory1";
	args[5]  = "-external:IExternalIncludeDirectory2";
	args[6]  = "/std:c++latest";
	args[7]  = "/std:c++23preview";
	args[8]  = "/std:c++11";
	args[9]  = "-std:c++14";
	args[10] = "/FIIncludeFile1";
	args[11] = "-FIIncludeFile2";
	args[12] = "/SomeUnknownOption";

	replaceMsvcFlags(&args);
	REQUIRE(args.size() == 12);
	REQUIRE(args[0]  == "C:\\some_path\\cl.exe");
	REQUIRE(args[1]  == "-DDefine");
	REQUIRE(args[2]  == "-UUndefine");
	REQUIRE(args[3]  == "-IIncludeDirectory");
	REQUIRE(args[4]  == "-isystem ExternalIncludeDirectory1");
	REQUIRE(args[5]  == "-isystem ExternalIncludeDirectory2");
	REQUIRE(args[6]  == "-std=c++23");
	REQUIRE(args[7]  == "-std=c++2c");
	REQUIRE(args[8]  == "-std=c++11");
	REQUIRE(args[9]  == "-std=c++14");
	REQUIRE(args[10] == "-include IncludeFile1");
	REQUIRE(args[11] == "-include IncludeFile2");
	// Removed: "/SomeUnknownOption"
}

