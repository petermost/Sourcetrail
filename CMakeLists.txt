cmake_minimum_required(VERSION 3.27)

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

get_property(isMultiConfigGenerator GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if (isMultiConfigGenerator)
	message(FATAL_ERROR "TODO: Multi config generator support is not tested!")
	foreach(CONFIGURATION_TYPE ${CMAKE_CONFIGURATION_TYPES})
		file(COPY "bin/app/data/"  DESTINATION "${CONFIGURATION_TYPE}/app/data/")
		file(COPY "bin/app/user/"  DESTINATION "${CONFIGURATION_TYPE}/app/user/")
		file(COPY "bin/test/data/" DESTINATION "${CONFIGURATION_TYPE}/test/data/")
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
	endif()
	add_library(External_lib_sqlite3 ALIAS unofficial::sqlite3::sqlite3)
else()
	find_package(SQLite3 REQUIRED)
	add_library(External_lib_sqlite3 ALIAS SQLite::SQLite3)
endif()
message(STATUS "Found SQLite3 ${SQLite3_VERSION}")

# TinyXML ----------------------------------------------------------------------

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
endif()
message(STATUS "Found tinyxml ${tinyxml_VERSION}")

# AidKit  -----------------------------------------------------------------------------------

add_subdirectory(src/lib_aidkit)

# External Lib CppSQlite3  ------------------------------------------------------------------

add_subdirectory(src/external)
# message(STATUS "Using CppSQLite3 ${CppSQLite3_VERSION}")

# Lib Utility ------------------------------------------------------------------

add_subdirectory(src/lib_utility)

# Lib Gui ----------------------------------------------------------------------

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

	# Clang:

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

# Indexer App ------------------------------------------------------------------

add_subdirectory(src/indexer)

# App --------------------------------------------------------------------------

add_subdirectory(src/app)

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
