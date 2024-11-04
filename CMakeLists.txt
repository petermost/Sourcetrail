<<<<<<< HEAD
cmake_minimum_required(VERSION 3.27)
=======
cmake_minimum_required(VERSION 3.29)
>>>>>>> 09ccbe42a1120f7185e91e13d9d2b8583217be7f

# Detect vcpkg build scenario here, so we don't depend on CMakePresets settings:
if (CMAKE_TOOLCHAIN_FILE MATCHES "vcpkg.cmake$")
	message(STATUS "Vcpkg Build")
	set(isVcpkgBuild TRUE)
else()
	message(STATUS "System Build")
	set(isVcpkgBuild FALSE)
endif()

# Make the build type available as a compiler variable:
if (isVcpkgBuild)
	add_compile_definitions(IS_VCPKG_BUILD=1)
else()
	add_compile_definitions(IS_VCPKG_BUILD=0)
endif()

include(CMakePrintHelpers)

# Configure which language(s)/package(s) should be supported:
option(BUILD_CXX_LANGUAGE_PACKAGE    "Add C/C++ support to the Sourcetrail indexer." OFF)
option(BUILD_JAVA_LANGUAGE_PACKAGE   "Add Java support to the Sourcetrail indexer." OFF)
option(BUILD_PYTHON_LANGUAGE_PACKAGE "Add Python support to the Sourcetrail indexer." OFF)
option(BUILD_UNIT_TESTS_PACKAGE      "Build the corresponding language unit tests." OFF)

configure_file(cmake/language_packages.h.in src/lib/language_packages.h)

# prohibit in-source-builds
if (${CMAKE_BINARY_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
	message(FATAL_ERROR "In-source-builds are strongly discouraged and not supported!")
endif()

# Make Ninja build verbose as well (https://github.com/ninja-build/ninja/issues/900):
if (CMAKE_VERBOSE_MAKEFILE)
	set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "Verbose Makefile" FORCE)
endif()

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Project ----------------------------------------------------------------------

project(Sourcetrail VERSION 2024.9.23)
configure_file(cmake/productVersion.h.in src/lib_gui/productVersion.h)

include(cmake/Sourcetrail.cmake)

# set Standard build type to Release
set(CMAKE_BUILD_TYPE_INIT "Release")

#[[
#RPATH
if (UNIX)
	set(CMAKE_SKIP_BUILD_RPATH FALSE)
	set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
	set(CMAKE_INSTALL_RPATH "$ORIGIN/lib/:$$ORIGIN/lib/")
endif()
]]

# Specify the requested standards:
# https://cmake.org/cmake/help/latest/prop_tgt/C_STANDARD.html
# https://cmake.org/cmake/help/latest/prop_tgt/CXX_STANDARD.html

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Prepare Data ----------------------------------------------------------------

<<<<<<< HEAD
get_property(isMultiConfigGenerator GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if (isMultiConfigGenerator)
	message(FATAL_ERROR "TODO: Multi config generator support is not tested!")
	foreach(CONFIGURATION_TYPE ${CMAKE_CONFIGURATION_TYPES})
		file(COPY "bin/app/data/"  DESTINATION "${CONFIGURATION_TYPE}/app/data/")
		file(COPY "bin/app/user/"  DESTINATION "${CONFIGURATION_TYPE}/app/user/")
		file(COPY "bin/test/data/" DESTINATION "${CONFIGURATION_TYPE}/test/data/")
=======
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-warning-option -fcolor-diagnostics -fvisibility-inlines-hidden")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	ADD_COMPILE_OPTIONS(/MP)
endif()

# FIXME: necessary to fix build in Docker
if (DOCKER_BUILD)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++")
endif()

if (TREAT_WARNINGS_AS_ERRORS)
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
		# Visual Studio 2017 version 15.9 <= Version <= Visual Studio 2019 Version 16.4
		if ((MSVC_VERSION GREATER_EQUAL 1916) AND (MSVC_VERSION LESS_EQUAL 1924))
			# Warning 4003: not enough actual parameters for macro 'identifier'
			# Warning 4250: 'class1' inherits 'class2::member' via dominance
			set(WARNINGS_LIST "/wd4003 /wd4250")
			set(CMAKE_CXX_WARNINGS_FLAGS "/experimental:external /external:anglebrackets /external:W0 /WX")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_WARNINGS_FLAGS} ${WARNINGS_LIST}")

			# Treat linker warnings as errors
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /WX")
			set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /WX")
			set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /WX")
			set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /WX")

			message(STATUS "'Treat warnings as errors' policy is enabled")
		else()
			message(STATUS "'Treat warnings as errors' policy is disabled")
		endif()
	else()
		message(STATUS "'Treat warnings as errors' policy is disabled")
	endif()
endif()

# For debugging the release build on linux
#if (UNIX AND "${CMAKE_BUILD_TYPE}" STREQUAL "Release")
	#add_definitions(-fno-omit-frame-pointer)
#endif ()


# Clang ------------------------------------------------------------------------

if (BUILD_CXX_LANGUAGE_PACKAGE)

	find_package(Clang REQUIRED)

	if (LLVM_FOUND)
		message(STATUS "Found LLVM ${LLVM_VERSION}")
	endif()

	if (UNIX)
		set(LLVM_CONFIGURATION_TYPES ".")
	endif()

	set (CLANG_COMPILER_HEADER_SEARCH_PATH "")
	foreach(LLVM_CONFIGURATION_TYPE ${LLVM_CONFIGURATION_TYPES})
		if (CLANG_COMPILER_HEADER_SEARCH_PATH STREQUAL "")
			set (_CLANG_HEADERS_SEARCH_LIST
				"${LLVM_BINARY_DIR}/${LLVM_CONFIGURATION_TYPE}/lib/clang/${LLVM_VERSION}/include"
				"${LLVM_BINARY_DIR}/${LLVM_CONFIGURATION_TYPE}/lib64/clang/${LLVM_VERSION}/include"
				"${LLVM_TOOLS_BINARY_DIR}/../lib/clang/${LLVM_VERSION_MAJOR}/include/"
				"${LLVM_TOOLS_BINARY_DIR}/../lib64/clang/${LLVM_VERSION_MAJOR}/include/"
			)
			foreach (_CLANG_HEADER_PATH ${_CLANG_HEADERS_SEARCH_LIST})
				if (EXISTS ${_CLANG_HEADER_PATH})
					set (CLANG_COMPILER_HEADER_SEARCH_PATH ${_CLANG_HEADER_PATH})
					break ()
				endif ()
			endforeach ()

			message (STATUS "Trying to find Clang compiler headers in '${LLVM_CONFIGURATION_TYPE}' build config in directory '${CLANG_COMPILER_HEADER_SEARCH_PATH}'.")
			if (EXISTS ${CLANG_COMPILER_HEADER_SEARCH_PATH})
				message (STATUS "Found headers for '${LLVM_CONFIGURATION_TYPE}' build config.")
				file(GLOB_RECURSE CLANG_COMPILER_HEADER_PATHS RELATIVE "${CLANG_COMPILER_HEADER_SEARCH_PATH}" "${CLANG_COMPILER_HEADER_SEARCH_PATH}/*")
				foreach(CLANG_COMPILER_HEADER_PATH ${CLANG_COMPILER_HEADER_PATHS})
					configure_file("${CLANG_COMPILER_HEADER_SEARCH_PATH}/${CLANG_COMPILER_HEADER_PATH}" "${CMAKE_SOURCE_DIR}/bin/app/data/cxx/include/${CLANG_COMPILER_HEADER_PATH}" COPYONLY)
				endforeach()
			else ()
				set (CLANG_COMPILER_HEADER_SEARCH_PATH "")
			endif ()
		endif ()
>>>>>>> 09ccbe42a1120f7185e91e13d9d2b8583217be7f
	endforeach()
else()
	message(STATUS "Copying: ${CMAKE_SOURCE_DIR}/bin/app/ -> ${CMAKE_BINARY_DIR}/app/")
	file(COPY "${CMAKE_SOURCE_DIR}/bin/app/" DESTINATION "${CMAKE_BINARY_DIR}/app/")

	if (BUILD_UNIT_TESTS_PACKAGE)
		# The test data contain symlinked files and directories which are used/tested from
		# 'FilePathTestSuite' and 'FileSystemTestSuite'. CMake, as of this writing (2024-02-23),
		# doesn't copy symlinks correctly (https://gitlab.kitware.com/cmake/cmake/-/issues/14609)
		# at least under Windows, but under Linux it seems to copy correctly. But it seems
		# that Windows also doesn't handle symlinks very well and this is probably the reason why
		# the orignal developers had disabled the tests for Windows.

		message(STATUS "Copying: ${CMAKE_SOURCE_DIR}/bin/test/data/ -> ${CMAKE_BINARY_DIR}/test/data")
		file(COPY "${CMAKE_SOURCE_DIR}/bin/test/data/" DESTINATION "${CMAKE_BINARY_DIR}/test/data/")

		#[[ In case symlinking the test data is better, then here is the code for it, but be aware that
		the temporary test files will then show up in the version control!

		message(STATUS "Linking: ${CMAKE_SOURCE_DIR}/bin/test/data/ -> ${CMAKE_BINARY_DIR}/test/data")
		file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/test")
		file(CREATE_LINK "${CMAKE_SOURCE_DIR}/bin/test/data/" "${CMAKE_BINARY_DIR}/test/data" SYMBOLIC)
		]]
	endif()
endif()

# Boost ------------------------------------------------------------------------

set(Boost_NO_WARN_NEW_VERSIONS ON)

if (isVcpkgBuild)
	# TODO: Investigate bug entry: https://gitlab.kitware.com/cmake/cmake/-/issues/21200 under Linux.
	set(Boost_NO_SYSTEM_PATHS ON)
endif()

find_package(Boost 1.83 CONFIG REQUIRED COMPONENTS
	# compiled libraries:
	system program_options filesystem date_time locale

	# 'header-only' libraries which can't be found with find_package:
	# interprocess uuid asio process predef dll

	# See https://github.com/Kitware/CMake/blob/master/Modules/FindBoost.cmake#L1409
	# for a list of compiled libraries.
)
message(STATUS "Found Boost ${Boost_VERSION}")

add_library(External_lib_boost INTERFACE)

target_compile_definitions(External_lib_boost
	INTERFACE
	# Fix "Boost-uuid should link against bcrypt on windows"
		# (https://github.com/microsoft/vcpkg/issues/4481)
		BOOST_UUID_FORCE_AUTO_LINK

		# If boost::filesystem gets replaced with std::filesystem, then this symbol must be defined:
		#BOOST_DLL_USE_STD_FS

		# Trying to add the definition to the boost target like this:
		# target_compile_definitions(Boost::uuid PUBLIC BOOST_UUID_FORCE_AUTO_LINK)
		# leads to the error:
		# "Cannot specify compile definitions for target "Boost::uuid" which is not built by this project."
)

# Qt ---------------------------------------------------------------------------

<<<<<<< HEAD
set (QT_MIN_VERSION "6.4.2")
set (QT_MIN_VERSION_HEX 0x060402)
find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS
	Widgets PrintSupport Network Svg Core5Compat
)
message(STATUS "Found Qt ${Qt6_VERSION}")

# SQLite3 ----------------------------------------------------------------------

if (isVcpkgBuild)
	find_package(unofficial-sqlite3 CONFIG REQUIRED)
	if (NOT DEFINED SQLite3_VERSION)
		# Use quotes to indicate a version literal:
		set(SQLite3_VERSION "\"3.46.0#2\"")
=======
set (QT_MIN_VERSION "6.7.1")
set (QT_MIN_VERSION_HEX 0x060701)
find_package(Qt6 ${QT_MIN_VERSION} COMPONENTS Widgets PrintSupport Network Svg Core5Compat REQUIRED)

if (WIN32)
	find_package(Qt6 ${QT_MIN_VERSION} COMPONENTS WinExtras REQUIRED)
endif()

if(Qt6Widgets_FOUND)
	message(STATUS "Found Qt ${Qt6Widgets_VERSION}")

	# FIX: Qt was built with -reduce-relocations
	if (Qt6_POSITION_INDEPENDENT_CODE)
		SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
>>>>>>> 09ccbe42a1120f7185e91e13d9d2b8583217be7f
	endif()
	add_library(External_lib_sqlite3 ALIAS unofficial::sqlite3::sqlite3)
else()
	find_package(SQLite3 REQUIRED)
	add_library(External_lib_sqlite3 ALIAS SQLite::SQLite3)
endif()
message(STATUS "Found SQLite3 ${SQLite3_VERSION}")

# TinyXML ----------------------------------------------------------------------

<<<<<<< HEAD
if (isVcpkgBuild)
	find_package(tinyxml CONFIG REQUIRED)
	if (NOT DEFINED tinyxml_VERSION)
		# Use quotes to indicate a version literal:
		set(tinyxml_VERSION "\"2.6.2#10\"")
	endif()
	add_library(External_lib_tinyxml ALIAS unofficial-tinyxml::unofficial-tinyxml)
else()
	if (UNIX)
		find_package(PkgConfig REQUIRED)
	endif()
	pkg_check_modules(tinyxml tinyxml IMPORTED_TARGET REQUIRED)
	add_library(External_lib_tinyxml ALIAS PkgConfig::tinyxml)
=======
# Setup ---------------------------------------------------------------------------

if (WIN32)
	get_filename_component(QT_BINARY_DIR "${QT_MOC_EXECUTABLE}" PATH)
	set(QT_PLUGINS_DIR "${QT_BINARY_DIR}/../plugins")

	function(COPY_QT_BINARIES IS_DEBUG IS_APP)
		set(SUFFIX "")
		if (IS_DEBUG)
			set(SUFFIX "d")
		endif()

		set(CONFIGURATION "Release")
		if (IS_DEBUG)
			set(CONFIGURATION "Debug")
		endif()

		if(CMAKE_CL_64)
			set(BITS "64")
		else()
			set(BITS "32")
		endif()

		set(TARGET "test")
		if (IS_APP)
			set(TARGET "app")

			file(GLOB MY_PUBLIC_HEADERS
				"${CMAKE_SOURCE_DIR}/setup/dynamic_libraries/win${BITS}/app/${CONFIGURATION}/*"
			)
			file(COPY ${MY_PUBLIC_HEADERS} DESTINATION "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/")

		endif()

		configure_file("${QT_BINARY_DIR}/Qt6Core${SUFFIX}.dll" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/Qt6Core${SUFFIX}.dll" COPYONLY)
		configure_file("${QT_BINARY_DIR}/Qt6Gui${SUFFIX}.dll" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/Qt6Gui${SUFFIX}.dll" COPYONLY)
		configure_file("${QT_BINARY_DIR}/Qt6Network${SUFFIX}.dll" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/Qt6Network${SUFFIX}.dll" COPYONLY)
		configure_file("${QT_BINARY_DIR}/Qt6Svg${SUFFIX}.dll" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/Qt6Svg${SUFFIX}.dll" COPYONLY)
		configure_file("${QT_BINARY_DIR}/Qt6Widgets${SUFFIX}.dll" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/Qt6Widgets${SUFFIX}.dll" COPYONLY)
		configure_file("${QT_BINARY_DIR}/Qt6Core5Compat${SUFFIX}.dll" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/Qt6Core5Compat${SUFFIX}.dll" COPYONLY)
		configure_file("${QT_BINARY_DIR}/Qt6WinExtras${SUFFIX}.dll" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/Qt6WinExtras${SUFFIX}.dll" COPYONLY)

		configure_file("${QT_PLUGINS_DIR}/platforms/qwindows${SUFFIX}.dll" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/platforms/qwindows${SUFFIX}.dll" COPYONLY)

		configure_file("${QT_PLUGINS_DIR}/styles/qwindowsvistastyle${SUFFIX}.dll" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/styles/qwindowsvistastyle${SUFFIX}.dll" COPYONLY)

		configure_file("${QT_PLUGINS_DIR}/imageformats/qgif${SUFFIX}.dll" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/imageformats/qgif${SUFFIX}.dll" COPYONLY)
		configure_file("${QT_PLUGINS_DIR}/imageformats/qicns${SUFFIX}.dll" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/imageformats/qicns${SUFFIX}.dll" COPYONLY)
		configure_file("${QT_PLUGINS_DIR}/imageformats/qico${SUFFIX}.dll" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/imageformats/qico${SUFFIX}.dll" COPYONLY)
		configure_file("${QT_PLUGINS_DIR}/imageformats/qjpeg${SUFFIX}.dll" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/imageformats/qjpeg${SUFFIX}.dll" COPYONLY)
		configure_file("${QT_PLUGINS_DIR}/imageformats/qsvg${SUFFIX}.dll" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/imageformats/qsvg${SUFFIX}.dll" COPYONLY)
		configure_file("${QT_PLUGINS_DIR}/imageformats/qtga${SUFFIX}.dll" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/imageformats/qtga${SUFFIX}.dll" COPYONLY)
		configure_file("${QT_PLUGINS_DIR}/imageformats/qtiff${SUFFIX}.dll" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/imageformats/qtiff${SUFFIX}.dll" COPYONLY)
		configure_file("${QT_PLUGINS_DIR}/imageformats/qwbmp${SUFFIX}.dll" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/imageformats/qwbmp${SUFFIX}.dll" COPYONLY)
		configure_file("${QT_PLUGINS_DIR}/imageformats/qwebp${SUFFIX}.dll" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/${TARGET}/imageformats/qwebp${SUFFIX}.dll" COPYONLY)
	endfunction(COPY_QT_BINARIES)

	COPY_QT_BINARIES(True True)
	COPY_QT_BINARIES(True False)
	COPY_QT_BINARIES(False True)
	COPY_QT_BINARIES(False False)

	configure_file("${CMAKE_SOURCE_DIR}/setup/icon/windows/sourcetrail.ico" "${CMAKE_BINARY_DIR}/${CONFIGURATION}/sourcetrail.ico" COPYONLY)

>>>>>>> 09ccbe42a1120f7185e91e13d9d2b8583217be7f
endif()
message(STATUS "Found tinyxml ${tinyxml_VERSION}")

# AidKit  -----------------------------------------------------------------------------------

add_subdirectory(src/lib_aidkit)

# External Lib CppSQlite3  ------------------------------------------------------------------

add_subdirectory(src/external)
# message(STATUS "Using CppSQLite3 ${CppSQLite3_VERSION}")

# Lib Utility ------------------------------------------------------------------

add_subdirectory(src/lib_utility)

<<<<<<< HEAD
# Lib Gui ----------------------------------------------------------------------
=======
create_source_groups(${LIB_UTILITY_FILES})

set_property(
	TARGET ${LIB_UTILITY_PROJECT_NAME}
	PROPERTY INCLUDE_DIRECTORIES
		"${LIB_UTILITY_INCLUDE_PATHS}"
)

target_include_directories(${LIB_UTILITY_PROJECT_NAME} SYSTEM
	PUBLIC ${Boost_INCLUDE_DIR}
)

target_link_libraries(${LIB_UTILITY_PROJECT_NAME} ${Boost_LIBRARIES} Qt6::Widgets Qt6::Network Qt6::Core5Compat)

if (UNIX AND NOT APPLE)
	find_package(Threads REQUIRED)
	target_link_libraries(${LIB_UTILITY_PROJECT_NAME} ${CMAKE_DL_LIBS} rt ${CMAKE_THREAD_LIBS_INIT})
endif()

if (WIN32)
	target_link_libraries(${LIB_UTILITY_PROJECT_NAME} Qt6::WinExtras)
endif()
>>>>>>> 09ccbe42a1120f7185e91e13d9d2b8583217be7f

add_subdirectory(src/lib_gui)

# Lib --------------------------------------------------------------------------

if (UNIX)
	find_package(Threads REQUIRED)
endif()

add_subdirectory(src/lib)

#
# Resolve cyclic dependencies between Sourcetrail_lib and Sourcetrail_lib_gui:
#

if (NOT isVcpkgBuild)
	set_target_properties(Sourcetrail_lib
		PROPERTIES
		LINK_INTERFACE_MULTIPLICITY 3
	)

set_target_properties(Sourcetrail_lib_gui
		PROPERTIES
		LINK_INTERFACE_MULTIPLICITY 3
	)
endif()

# Lib Cxx ----------------------------------------------------------------------

if (BUILD_CXX_LANGUAGE_PACKAGE)

<<<<<<< HEAD
	# Clang:
=======
	set(CAPTURED_CMAKE_CXX_STANDARD ${CMAKE_CXX_STANDARD})
	set(CMAKE_CXX_STANDARD 17)
	
	add_library(${LIB_CXX_PROJECT_NAME} ${LIB_CXX_FILES})
>>>>>>> 09ccbe42a1120f7185e91e13d9d2b8583217be7f

	find_package(Clang 18.1...<19.0 REQUIRED)
	message(STATUS "Found LLVM ${LLVM_VERSION}")

	if (isVcpkgBuild)
		set(headerSourceDir "${LLVM_TOOLS_BINARY_DIR}/lib/clang/${LLVM_VERSION_MAJOR}/include/")
	else()
		set(headerSourceDir "${LLVM_TOOLS_BINARY_DIR}/../lib/clang/${LLVM_VERSION_MAJOR}/include/")
	endif()
	set(headerTargetDir "app/data/cxx/include/")

	message(STATUS "Copying LLVM header: ${headerSourceDir} -> ${headerTargetDir}")
	file(COPY "${headerSourceDir}" DESTINATION "${headerTargetDir}")

	add_library(External_lib_clang INTERFACE)
	target_compile_definitions(External_lib_clang
		INTERFACE
		${LLVM_DEFINITIONS}
	)
target_include_directories(External_lib_clang SYSTEM
		INTERFACE
		${LLVM_INCLUDE_DIRS}
	)
llvm_map_components_to_libnames(REQ_LLVM_LIBS
		${LLVM_TARGETS_TO_BUILD} support core libdriver passes option
	)
target_link_libraries(External_lib_clang
		INTERFACE
		clangASTMatchers
			clangFrontend
			clangSerialization
			clangDriver
			clangTooling
			clangParse
			clangSema
			clangStaticAnalyzerFrontend
			clangStaticAnalyzerCheckers
			clangStaticAnalyzerCore
			clangAnalysis
			clangRewriteFrontend
			clangEdit
			clangAST
			clangLex
			clangBasic

			${REQ_LLVM_LIBS}
	)

add_subdirectory(src/lib_cxx)
else()
	message(STATUS "Building the Cxx indexer will be skipped. You can enable building this target by setting 'BUILD_CXX_LANGUAGE_PACKAGE' to 'ON'.")
endif()

# Lib Java ---------------------------------------------------------------------

if (BUILD_JAVA_LANGUAGE_PACKAGE)
	# Java:
	find_package(Java 1.8 REQUIRED)
	message(STATUS "Found Java ${Java_VERSION}")

	# JNI:
	find_package(JNI REQUIRED)
	message(STATUS "Found JNI ${JNI_VERSION}")

	# Maven:
	find_program(MVN_COMMAND NAMES "mvn" REQUIRED)
	message(STATUS "Found Maven ${MVN_COMMAND}")

	add_subdirectory(src/lib_java)
else()
	message(STATUS "Building the Java indexer will be skipped. You can enable building this target by setting 'BUILD_JAVA_LANGUAGE_PACKAGE' to 'ON'.")
endif()

# Lib Python -------------------------------------------------------------------

if (BUILD_PYTHON_LANGUAGE_PACKAGE)
	# Bash:
	find_program(BASH_COMMAND NAMES "bash" REQUIRED)
	message(STATUS "Found Bash ${BASH_COMMAND}")

	add_subdirectory(src/lib_python)
else()
	message(STATUS "Building the Python indexer will be skipped. You can enable building this target by setting 'BUILD_PYTHON_LANGUAGE_PACKAGE' to 'ON'.")
endif()

<<<<<<< HEAD
=======

# Lib Gui ----------------------------------------------------------------------

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# target for running versionnumber script
# workaround for running customcommand (ninja dependency cycle)
add_custom_target(
	versionnumber ALL
)

add_library(${LIB_GUI_PROJECT_NAME} ${LIB_GUI_FILES} ${CMAKE_BINARY_DIR}/src/lib_gui/productVersion.h)

target_link_libraries(${LIB_GUI_PROJECT_NAME} ${LIB_UTILITY_PROJECT_NAME} ${LIB_PROJECT_NAME} Qt6::Widgets Qt6::Network Qt6::Svg Qt6::Core5Compat)

if (WIN32)
	target_link_libraries(${LIB_GUI_PROJECT_NAME} Qt6::WinExtras)
endif()

# command for versioning script
add_custom_command(
	TARGET versionnumber
	PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -DBINARY_DIR=${CMAKE_BINARY_DIR} -P ${CMAKE_SOURCE_DIR}/cmake/version.cmake
	BYPRODUCTS ${CMAKE_BINARY_DIR}/src/lib_gui/productVersion.h
	DEPENDS ${LIB_GUI_PROJECT_NAME}
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	COMMENT "check/update version number"
)
add_dependencies(${LIB_GUI_PROJECT_NAME} versionnumber)

create_source_groups(${LIB_GUI_FILES})

set_property(
	TARGET ${LIB_GUI_PROJECT_NAME}
	PROPERTY INCLUDE_DIRECTORIES
		"${LIB_GUI_INCLUDE_PATHS}"
		"${LIB_UTILITY_INCLUDE_PATHS}"
		"${LIB_INCLUDE_PATHS}"
		"${CMAKE_BINARY_DIR}/src/lib_gui"
		"${CMAKE_BINARY_DIR}/src/lib"
		$<$<BOOL:${BUILD_CXX_LANGUAGE_PACKAGE}>:${LIB_CXX_INCLUDE_PATHS}>
		$<$<BOOL:${BUILD_JAVA_LANGUAGE_PACKAGE}>:${LIB_JAVA_INCLUDE_PATHS}>
		$<$<BOOL:${BUILD_PYTHON_LANGUAGE_PACKAGE}>:${LIB_PYTHON_INCLUDE_PATHS}>
)

# include external header without warnings
target_include_directories(${LIB_GUI_PROJECT_NAME} SYSTEM
	PUBLIC ${Boost_INCLUDE_DIRS}
	"${EXTERNAL_INCLUDE_PATHS}"
	"${EXTERNAL_C_INCLUDE_PATHS}"
)

# configure platform specific include file
configure_file(
	"${PROJECT_SOURCE_DIR}/src/lib_gui/platform_includes/includes.h.in"
	"${PROJECT_BINARY_DIR}/src/lib_gui/includes.h"
)

#configure the versioning file
configure_file(
	${CMAKE_SOURCE_DIR}/cmake/version.txt.in
	${CMAKE_BINARY_DIR}/version.txt
)

configure_file(
	${CMAKE_SOURCE_DIR}/cmake/productVersion.h.in
	${CMAKE_BINARY_DIR}/src/lib_gui/productVersion.h
)

set_property(SOURCE ${CMAKE_BINARY_DIR}/src/lib_gui/productVersion.h PROPERTY SKIP_AUTOMOC ON)

set(CMAKE_AUTOMOC OFF)


>>>>>>> 09ccbe42a1120f7185e91e13d9d2b8583217be7f
# Indexer App ------------------------------------------------------------------

add_subdirectory(src/indexer)

# App --------------------------------------------------------------------------

<<<<<<< HEAD
add_subdirectory(src/app)
=======
if (UNIX)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/app/")
else ()
	foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
		string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
		set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_BINARY_DIR}/${OUTPUTCONFIG}/app/")
	endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )
endif ()

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (WIN32)
	file(WRITE ${CMAKE_BINARY_DIR}/Sourcetrail.rc
		"// Icon with lowest ID value placed first to ensure application icon\n"
		"// remains consistent on all systems.\n"
		"IDI_ICON1               ICON                    \"${CMAKE_BINARY_DIR}/Sourcetrail.ico\"\n"
	)

	add_executable(${APP_PROJECT_NAME} ${APP_FILES} ${CMAKE_BINARY_DIR}/Sourcetrail.rc)

	# also show the console when running a release build.
	set_target_properties(${APP_PROJECT_NAME} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE /DEBUG:FASTLINK")
	set_target_properties(${APP_PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS_DEBUG "_CONSOLE")
	set_target_properties(${APP_PROJECT_NAME} PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
	set_target_properties(${APP_PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "_CONSOLE")
	set_target_properties(${APP_PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/ENTRY:\"mainCRTStartup\" /SUBSYSTEM:CONSOLE /DEBUG")
	set_target_properties(${APP_PROJECT_NAME} PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:CONSOLE")

	# generate pdb for release build
	set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")

	set_property(
		TARGET ${APP_PROJECT_NAME}
			PROPERTY VS_DEBUGGER_WORKING_DIRECTORY
			"${CMAKE_SOURCE_DIR}/bin/app")

	string(REGEX REPLACE "/" "\\\\" BACKSLASHED_CMAKE_BINARY_DIR ${CMAKE_BINARY_DIR})
	string(REGEX REPLACE "/" "\\\\" BACKSLASHED_CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR})
else ()
	add_executable(${APP_PROJECT_NAME} ${APP_FILES})
endif ()

create_source_groups(${APP_FILES})

target_link_libraries(
	${APP_PROJECT_NAME}
	${LIB_GUI_PROJECT_NAME}
	$<$<BOOL:${BUILD_CXX_LANGUAGE_PACKAGE}>:${LIB_CXX_PROJECT_NAME}>
	$<$<BOOL:${BUILD_JAVA_LANGUAGE_PACKAGE}>:${LIB_JAVA_PROJECT_NAME}>
	$<$<BOOL:${BUILD_PYTHON_LANGUAGE_PACKAGE}>:${LIB_PYTHON_PROJECT_NAME}>
	${LIB_PROJECT_NAME}
)

set_property(
	TARGET ${APP_PROJECT_NAME}
	PROPERTY INCLUDE_DIRECTORIES
		"${APP_INCLUDE_PATHS}"
		"${LIB_INCLUDE_PATHS}"
		"${LIB_UTILITY_INCLUDE_PATHS}"
		"${LIB_GUI_INCLUDE_PATHS}"
		"${CMAKE_BINARY_DIR}/src/lib_gui"
		"${CMAKE_BINARY_DIR}/src/lib"
		$<$<BOOL:${BUILD_CXX_LANGUAGE_PACKAGE}>:${LIB_CXX_INCLUDE_PATHS}>
		$<$<BOOL:${BUILD_JAVA_LANGUAGE_PACKAGE}>:${LIB_JAVA_INCLUDE_PATHS}>
		$<$<BOOL:${BUILD_PYTHON_LANGUAGE_PACKAGE}>:${LIB_PYTHON_INCLUDE_PATHS}>
)

target_include_directories(${APP_PROJECT_NAME} SYSTEM
	PUBLIC ${Boost_INCLUDE_DIRS}
	"${EXTERNAL_INCLUDE_PATHS}"
	"${EXTERNAL_C_INCLUDE_PATHS}"
)

# Use the Widgets module from Qt 6.
target_link_libraries(${APP_PROJECT_NAME} Qt6::Widgets Qt6::Network Qt6::Core5Compat)

# add platform specific libraries
if (APPLE)
	find_library(CORE_FOUNDATION CoreFoundation)
	target_link_libraries(${APP_PROJECT_NAME} ${CORE_FOUNDATION})
endif ()

add_dependencies(${APP_PROJECT_NAME} ${APP_INDEXER_NAME})


# macOS Bundle ----------------------------------------------------------------

if (APPLE)

	set(MACOS_BUNDLE_NAME ${PROJECT_NAME})
	set(MACOS_BUNDLE_VERSION ${VERSION_STRING})
	set(MACOS_BINARY_NAME ${APP_PROJECT_NAME})
	set(MACOS_INDEXER_BINARY_NAME ${APP_INDEXER_NAME})

	get_property(QT_CORE_PATH TARGET ${Qt6Core_LIBRARIES} PROPERTY LOCATION)
	get_filename_component(QT_CORE_PATH ${QT_CORE_PATH} REALPATH)

	get_property(QT_GUI_PATH TARGET ${Qt6Gui_LIBRARIES} PROPERTY LOCATION)
	get_filename_component(QT_GUI_PATH ${QT_GUI_PATH} REALPATH)

	get_property(QT_WIDGETS_PATH TARGET ${Qt6Widgets_LIBRARIES} PROPERTY LOCATION)
	get_filename_component(QT_WIDGETS_PATH ${QT_WIDGETS_PATH} REALPATH)

	get_property(QT_PRINT_PATH TARGET ${Qt6PrintSupport_LIBRARIES} PROPERTY LOCATION)
	get_filename_component(QT_PRINT_PATH ${QT_PRINT_PATH} REALPATH)

	get_property(QT_NETWORK_PATH TARGET ${Qt6Network_LIBRARIES} PROPERTY LOCATION)
	get_filename_component(QT_NETWORK_PATH ${QT_NETWORK_PATH} REALPATH)

	get_property(QT_SVG_PATH TARGET ${Qt6Svg_LIBRARIES} PROPERTY LOCATION)
	get_filename_component(QT_SVG_PATH ${QT_SVG_PATH} REALPATH)

	list(APPEND MACOS_QT_FRAMEWORKS ${QT_CORE_PATH} ${QT_GUI_PATH} ${QT_WIDGETS_PATH} ${QT_PRINT_PATH} ${QT_NETWORK_PATH} ${QT_SVG_PATH})
	string(REPLACE ";" " " MACOS_QT_FRAMEWORKS "${MACOS_QT_FRAMEWORKS}")

	set(MACOS_BOOST_DIR "${Boost_INCLUDE_DIR}")
	STRING(REGEX REPLACE "/lib/cmake/clang" "" MACOS_CLANG_DIR "${Clang_DIR}")
	STRING(REGEX REPLACE "/lib/cmake/Qt6" "" MACOS_QT_DIR "${Qt6_DIR}")

	configure_file(
		${PROJECT_SOURCE_DIR}/setup/macOS/bundle_install.sh.in
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/bundle_install.sh
		@ONLY
	)

	configure_file(
		${PROJECT_SOURCE_DIR}/setup/macOS/bundle_info.plist.in
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/bundle_info.plist
		@ONLY
	)

endif ()

>>>>>>> 09ccbe42a1120f7185e91e13d9d2b8583217be7f

# Test ----------------------------------------------------------------------

if (BUILD_UNIT_TESTS_PACKAGE)
	include(CTest)

	find_package(GTest CONFIG REQUIRED)
	message(STATUS "Found GTest ${GTest_VERSION}")
	include(GoogleTest)
	add_subdirectory(src/test_aidkit)

	find_package(Catch2 3.4 CONFIG REQUIRED)
	message(STATUS "Found Catch2 ${Catch2_VERSION}")
	include(Catch)
	add_subdirectory(src/test)
else()
	message(STATUS "Building the unit tests will be skipped. You can enable building this target by setting 'BUILD_UNIT_TESTS_PACKAGE' to 'ON'.")
endif()

# Create license information:
include(cmake/licenses.cmake)
configureLicenseFile(src/lib_gui/licenses.h)

# Installing/Packaging ---------------------------------------------------------

install(DIRECTORY "${CMAKE_BINARY_DIR}/app/" DESTINATION "${PROJECT_NAME}/app/" USE_SOURCE_PERMISSIONS)
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)

set(CPACK_PACKAGE_VERSION ${CMAKE_PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Source code explorer")
set(CPACK_PACKAGE_DESCRIPTION "Free and open-source cross-platform source explorer that helps you get productive on unfamiliar source code.")

set(CPACK_PACKAGE_CONTACT "P. Most <pmost@pera-software.com>")
set(CPACK_PACKAGE_VENDOR "PERA Software Solutions GmbH")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/petermost/Sourcetrail")
set(CPACK_PACKAGE_ICON "${CMAKE_BINARY_DIR}/app/Sourcetrail.ico")

if (isVcpkgBuild)
	set(CPACK_GENERATOR ZIP)
else()
	if (UNIX)
		set(CPACK_GENERATOR DEB)
		set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
		set(CPACK_PACKAGING_INSTALL_PREFIX "/opt/")
	else()
		set(CPACK_GENERATOR ZIP)
	endif()
endif()

include(CPack)
