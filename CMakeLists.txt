cmake_minimum_required(VERSION 3.27)

include(CMakePrintHelpers)

###############################################################################
# Vcpkg
###############################################################################

if (CMAKE_TOOLCHAIN_FILE MATCHES "vcpkg.cmake$")
	message(STATUS "Vcpkg Build")
	set(isVcpkgBuild TRUE)
else()
	message(STATUS "System Build")
	set(isVcpkgBuild FALSE)
endif()

if (isVcpkgBuild)
	set(VCPKG_INSTALL_OPTIONS
		# Don't print package usages:
		--no-print-usage

		# Prevent rebuilding after a compiler update:		
		--x-abi-tools-use-exact-versions
		
		# Clean everything except sources:
		#--clean-downloads-after-build
		#--clean-packages-after-build
	)
endif()

###############################################################################
# Project
###############################################################################

project(Sourcetrail VERSION 2026.6.3 HOMEPAGE_URL "http://sourcetrail.de")

include(cmake/Sourcetrail.cmake)

configureFile(cmake/productVersion.h.in src/lib/productVersion.h)

# Prohibit in-source-builds:

if (${CMAKE_BINARY_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
	message(FATAL_ERROR "In-source-builds are not supported!")
endif()

# Set default build type to 'Release':

set(defaultBuildType "Release")
set(defaultBuildConfigurations ${defaultBuildType} "Debug" "RelWithDebInfo" "MinSizeRel")
if (isMultiConfigGenerator)
	if (NOT CMAKE_DEFAULT_BUILD_TYPE)
		message(STATUS "Set multi-config default build type to 'Release'")
		set(CMAKE_DEFAULT_BUILD_TYPE ${defaultBuildType} CACHE STRING "Multi-config default build type" FORCE)

		# Present a combobox in the cmake-gui:
		set_property(CACHE CMAKE_DEFAULT_BUILD_TYPE PROPERTY STRINGS ${defaultBuildConfigurations})
	endif()
	if(NOT CMAKE_CONFIGURATION_TYPES)
		set(CMAKE_CONFIGURATION_TYPES ${defaultBuildConfigurations} CACHE STRING "Multi-config available build types" FORCE)
	endif()
else()
	# Adapted from https://www.kitware.com//cmake-and-the-default-build-type/
	if (NOT CMAKE_BUILD_TYPE)
		message(STATUS "Set single-config default build type to 'Release'")
		set(CMAKE_BUILD_TYPE ${defaultBuildType} CACHE STRING "Single-config default build type" FORCE)

		# Present a combobox in the cmake-gui:
		set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${defaultBuildConfigurations})
	endif()
endif()

# Make Ninja build verbose as well (https://github.com/ninja-build/ninja/issues/900):

if (CMAKE_VERBOSE_MAKEFILE)
	set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "Verbose Makefile" FORCE)
endif()

# Make the build type available as a compiler variable:

if (isVcpkgBuild)
	add_compile_definitions(IS_VCPKG_BUILD=1)
else()
	add_compile_definitions(IS_VCPKG_BUILD=0)
endif()

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find includes in corresponding build directories:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Use recommended optimization switches:

if (isGccCompiler OR isClangCompiler)
	# https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html
	# CMakeCache.txt entry: "-O3 -DNDEBUG"
	add_compile_options("$<$<CONFIG:Release>:-O3;-fno-fast-math;-flto=auto>")
	add_link_options("$<$<CONFIG:Release>:-flto=auto>")
	
	# Release-Build with Clang v21 on (K)Ubuntu v25.10 fails:
	# 1) `-flto=auto` fails with: "error loading plugin: /usr/lib/llvm-21/bin/../lib/LLVMgold.so: cannot open shared object file"
	# 2) `-fuse-ld=lld` fails with: "ld.lld: error: relocation refers to a discarded section: .text._ZN7testing4Test5SetupEv"
	#
	# Try to prevent build problems on Clang based distributions:
	if (isClangCompiler)
		add_link_options("$<$<CONFIG:Release>:-fuse-ld=lld>")
	endif()
elseif (isMsvcCompiler)
	# https://learn.microsoft.com/en-us/cpp/build/optimization-best-practices
	# CMakeCache.txt entry: "/O2 /Ob2 /DNDEBUG"
	add_compile_options("$<$<CONFIG:Release>:/O2;/Gy;/GL;/fp:precise;/GS->")
	add_link_options("$<$<CONFIG:Release>:/LTCG;/OPT:REF,ICF>")

	# For profiling:
	#add_compile_options("$<$<CONFIG:RELEASE>:/Zi>")
	#add_link_options("$<$<CONFIG:Release>:/DEBUG>")
endif()

###############################################################################
# Configure which language(s)/package(s) should be build
###############################################################################

option(BUILD_CXX_LANGUAGE_PACKAGE  "Add C/C++ support to the Sourcetrail indexer." OFF)
option(BUILD_JAVA_LANGUAGE_PACKAGE "Add Java support to the Sourcetrail indexer." OFF)
option(BUILD_UNIT_TESTS_PACKAGE    "Build the corresponding language unit tests." OFF)

configureFile(cmake/language_packages.h.in src/lib/language_packages.h)

# Some sanity checks:

if (NOT BUILD_CXX_LANGUAGE_PACKAGE)
	message(STATUS "Building the C++ indexer is disabled. You can enable it by setting 'BUILD_CXX_LANGUAGE_PACKAGE' to 'ON'.")
endif()

if (NOT BUILD_JAVA_LANGUAGE_PACKAGE)
	message(STATUS "Building the Java indexer is disabled. You can enable it by setting 'BUILD_JAVA_LANGUAGE_PACKAGE' to 'ON'.")
endif()

if (BUILD_UNIT_TESTS_PACKAGE)
	include(CTest)
else()
	message(STATUS "Building the tests is disabled. You can enable them by setting 'BUILD_UNIT_TESTS_PACKAGE' to 'ON'.")
endif()

if (NOT BUILD_CXX_LANGUAGE_PACKAGE AND NOT BUILD_JAVA_LANGUAGE_PACKAGE)
	message(WARNING "Building ONLY the GUI, because all language indexers are disabled!")
endif()

if (BUILD_PYTHON_LANGUAGE_PACKAGE)
	message(FATAL_ERROR "Python support has been removed!")
	# See:
	# - https://github.com/petermost/Sourcetrail/issues/34
	# - https://github.com/CoatiSoftware/SourcetrailPythonIndexer
endif()

#[[
#RPATH
if (UNIX)
	set(CMAKE_SKIP_BUILD_RPATH FALSE)
	set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
	set(CMAKE_INSTALL_RPATH "$ORIGIN/lib/:$$ORIGIN/lib/")
endif()
]]

# Ensure <library-v2.0> is found before <library-v1.0>:

set(CMAKE_FIND_PACKAGE_SORT_ORDER NATURAL)

###############################################################################
# Prepare Data
###############################################################################

function(copyGenerateCoverageReportScript configDir)
	if (UNIX)
		# copyFile("scripts/generate_coverage_report.sh" "${configDir}/generate_coverage_report.sh")
	endif()
endfunction()
forEachBuildConfiguration(copyGenerateCoverageReportScript)

function(copyAppData configDir)
	copyDirectory("bin/app/" "${configDir}/app/")
endfunction()
forEachBuildConfiguration(copyAppData)

if (BUILD_UNIT_TESTS_PACKAGE)
	# The test data contains symlinked files and directories which are used from 'FilePathTestSuite' 
	# and 'FileSystemTestSuite'. CMake, as of this writing (2025-03-07), doesn't copy symlinks 
	# correctly (https://gitlab.kitware.com/cmake/cmake/-/issues/14609) or rather it depends on the 
	# underlying platform.

	function(copyTestData configDir)	
		set(testDataSourceDir "bin/test/data/")
		set(testDataTargetDir "${configDir}/test/data") # Without a '/' at the end!
		
		if (UNIX)
			# Under Linux the symlinks are copied correctly, so copy the data:
			copyDirectory("${testDataSourceDir}" "${testDataTargetDir}/")
		else()
			# Under Windows the symlinks are *not* copied correctly (results in invalid files), so 
			# symlink the data.
			# Note: Temporary test files will show up in version control!
			symlinkDirectory("${testDataSourceDir}" "${testDataTargetDir}")
		endif()
	endfunction()
	forEachBuildConfiguration(copyTestData)
endif()

###############################################################################
# ICU
###############################################################################

find_package(ICU REQUIRED 
	COMPONENTS 
		i18n uc data
)
message(STATUS "Found ICU ${ICU_VERSION}: ${ICU_INCLUDE_DIRS}") # ${ICU_LIBRARIES}

add_library(External_lib_icu INTERFACE)

target_compile_definitions(External_lib_icu
	INTERFACE
		U_CHARSET_IS_UTF8=1
)

# The documented imported targets:
# ICU::i18n
# ICU::uc
# ICU::data
# lead to a linker error: "undefined reference to `icudt74_dat'", 
# so use the provided result variables:

target_include_directories(External_lib_icu
	INTERFACE
		${ICU_INCLUDE_DIRS}
)

target_link_libraries(External_lib_icu
	INTERFACE
		${ICU_LIBRARIES}
)
		
###############################################################################
# Boost / External_lib_boost
###############################################################################

set(Boost_NO_WARN_NEW_VERSIONS ON)

if (isVcpkgBuild)
	# Associated bug entry: https://gitlab.kitware.com/cmake/cmake/-/issues/21200 has been closed.
	set(Boost_NO_SYSTEM_PATHS ON)
endif()

set(BOOST_MIN_VERSION 1.83)
set(BOOST_MAX_VERSION 1.92)

find_package(Boost CONFIG REQUIRED 
	COMPONENTS
		date_time 
		filesystem 
		locale
		program_options 
)
message(STATUS "Found Boost ${Boost_VERSION}: ${Boost_INCLUDE_DIRS}") # ${Boost_DIR} contains the path to BoostConfig.cmake|boost-config.cmake.
checkVersionRange("Boost" ${Boost_VERSION} ${BOOST_MIN_VERSION} ${BOOST_MAX_VERSION})

add_library(External_lib_boost INTERFACE)

target_compile_definitions(External_lib_boost
	INTERFACE
		# Try to detect deprecations early:
		#BOOST_ASIO_NO_DEPRECATED
		#BOOST_PROCESS_NO_DEPRECATED
		#BOOST_FILESYSTEM_NO_DEPRECATED
		
		# Use the same linking approach on all platforms:
		#BOOST_ALL_NO_LIB
		
		# Fix "Boost-uuid should link against bcrypt on windows"
		# (https://github.com/microsoft/vcpkg/issues/4481)
		BOOST_UUID_FORCE_AUTO_LINK

		# If boost::filesystem gets replaced with std::filesystem, then this symbol must be defined:
		#BOOST_DLL_USE_STD_FS
)

target_link_libraries(External_lib_boost
	INTERFACE
		Boost::headers
		
		Boost::date_time
		Boost::filesystem
		Boost::locale
		Boost::program_options
		
		# Prevent linker error in vcpkg build:
		External_lib_icu
)

###############################################################################
# Qt / External_lib_qt
###############################################################################

set(QT_NO_PRIVATE_MODULE_WARNING ON)

set(QT_MIN_VERSION 6.8.3)
# set (QT_MAX_VERSION_HEX 0x060802)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED 
	COMPONENTS
		Widgets 
		Network 
		Svg
)
message(STATUS "Found Qt ${Qt6_VERSION}: ${Qt6Core_INCLUDE_DIRS}")

add_library(External_lib_qt INTERFACE)

# Enable warnings for deprecated APIs:
target_compile_definitions(External_lib_qt
	INTERFACE
		# QT_DISABLE_DEPRECATED_UP_TO=${QT_MAX_VERSION_HEX}
		# QT_ENABLE_STRICT_MODE_UP_TO=${QT_MAX_VERSION_HEX}
		QT_NO_FOREACH
		QT_NO_NARROWING_CONVERSIONS_IN_CONNECT
		QT_NO_QASCONST
		QT_USE_NODISCARD_FILE_OPEN
		QT_NO_QSNPRINTF
		
		# Prevent creating QPixmap inadvertently with 'const char []':
		QT_NO_IMAGEFORMAT_XPM
)

target_link_libraries(External_lib_qt
	INTERFACE
		Qt6::Widgets
		Qt6::Network
		Qt6::Svg
)

###############################################################################
# SQLite / External_lib_sqlite3 / External_lib_cppsqlite3
###############################################################################

if (isVcpkgBuild)
	find_package(unofficial-sqlite3 CONFIG REQUIRED)
	if (NOT DEFINED SQLite3_VERSION)
		# Use quotes to indicate a version literal:
		set(SQLite3_VERSION "\"3.53.1\"")
	endif()
	add_library(External_lib_sqlite3 ALIAS unofficial::sqlite3::sqlite3)
else()
	find_package(SQLite3 REQUIRED)
	add_library(External_lib_sqlite3 ALIAS SQLite::SQLite3)
endif()
message(STATUS "Found SQLite3 ${SQLite3_VERSION}")

add_subdirectory(src/external)
# message(STATUS "Using CppSQLite3 ${CppSQLite3_VERSION}")

###############################################################################
# TinyXML / External_lib_tinyxml
###############################################################################

if (isVcpkgBuild)
	find_package(tinyxml CONFIG REQUIRED)
	if (NOT DEFINED tinyxml_VERSION)
		# Use quotes to indicate a version literal:
		set(tinyxml_VERSION "\"2.6.2\"")
	endif()
	add_library(External_lib_tinyxml ALIAS unofficial-tinyxml::unofficial-tinyxml)
else()
	if (UNIX)
		find_package(PkgConfig REQUIRED)
	endif()
	pkg_check_modules(tinyxml tinyxml IMPORTED_TARGET REQUIRED)
	add_library(External_lib_tinyxml ALIAS PkgConfig::tinyxml)
endif()
message(STATUS "Found tinyxml ${tinyxml_VERSION}: ${tinyxml_INCLUDE_DIRS}")

###############################################################################
# AidKit / AidKit_lib / AidKit_test
###############################################################################

add_subdirectory(src/lib_aidkit)

if (BUILD_UNIT_TESTS_PACKAGE)
	find_package(GTest CONFIG REQUIRED)
	message(STATUS "Found GTest ${GTest_VERSION}")
	include(GoogleTest)

	add_subdirectory(src/test_aidkit)
endif()

###############################################################################
# Sourcetrail_lib_gui
###############################################################################

add_subdirectory(src/resources)
add_subdirectory(src/lib_gui)

###############################################################################
# Sourcetrail_lib
###############################################################################

if (UNIX)
	find_package(Threads REQUIRED)
endif()

add_subdirectory(src/lib)

#
# Resolve cyclic dependencies between Sourcetrail_lib and Sourcetrail_lib_gui:
#

set_target_properties(Sourcetrail_lib
	PROPERTIES
		LINK_INTERFACE_MULTIPLICITY 3
)
set_target_properties(Sourcetrail_lib_gui
	PROPERTIES
		LINK_INTERFACE_MULTIPLICITY 3
)

###############################################################################
# Clang / External_lib_clang
###############################################################################

if (BUILD_CXX_LANGUAGE_PACKAGE)

	# With the LLVM-overlay-port in place, we can use the exact same LLVM version 
	# for system- and vcpkg-build, so we don't accept a range anymore, but only a
	# single version.
	
	set(CLANG_MIN_VERSION 21)
	set(CLANG_MAX_VERSION 22)
	find_package(Clang CONFIG REQUIRED) # Minimum version and/or version range xx...<yy doesn't work.
	message(STATUS "Found LLVM: ${LLVM_VERSION}") # ${LLVM_INSTALL_PREFIX} ${CLANG_INSTALL_PREFIX}")
	checkVersionRange("Clang" ${LLVM_VERSION} ${CLANG_MIN_VERSION} ${CLANG_MAX_VERSION})

	add_library(External_lib_clang INTERFACE)

	# Integrate LLVM/LibTooling. 
	# For further details see https://llvm.org/docs/CMake.html#embedding-llvm-in-your-project
	
	separate_arguments(llvmDefinitions NATIVE_COMMAND ${LLVM_DEFINITIONS})
	target_compile_definitions(External_lib_clang
		INTERFACE
			${llvmDefinitions}
	)
	target_include_directories(External_lib_clang SYSTEM
		INTERFACE
			${LLVM_INCLUDE_DIRS}
	)
	
	# 'LLVM_LINK_LLVM_DYLIB' is set when LLVM has been build into a single shared library i.e. 'libLLVM'
	# 'CLANG_LINK_CLANG_DYLIB' is set when Clang has been build into a single shared library i.e. 'libclang-cpp'
	
	if (LLVM_LINK_LLVM_DYLIB AND CLANG_LINK_CLANG_DYLIB)
		target_link_libraries(External_lib_clang
			INTERFACE
				LLVM
				clang-cpp
		)
	else()
		llvm_map_components_to_libnames(llvmLibraries 
			${LLVM_TARGETS_TO_BUILD} 
			support 
			core 
			libdriver 
			passes 
			option
		)
		target_link_libraries(External_lib_clang
			INTERFACE
				${llvmLibraries}
				
				# LibTooling libraries:
				
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
			)
	endif()
	
	# Copy required header:
	# Regular vcpkg (non-overlay-port) path was: "${CLANG_INSTALL_PREFIX}/tools/llvm/lib/clang/${LLVM_VERSION_MAJOR}/include/"
	function(copyClangHeader configDir)
		copyDirectory("${LLVM_INSTALL_PREFIX}/lib/clang/${Clang_VERSION_MAJOR}/include/" "${configDir}/app/data/cxx/include/")
	endfunction()
	forEachBuildConfiguration(copyClangHeader)
	
	add_subdirectory(src/lib_cxx)
endif()

###############################################################################
# Sourcetrail_lib_java / Sourcetrail_java_indexer
###############################################################################

# To set the `JAVA_HOME` environment variable automatically on Linux, you can use this line:
# export JAVA_HOME=$(dirname $(dirname $(readlink -f $(which javac))))
# See https://www.xmodulo.com/set-java_home-environment-variable-linux.html for further details

if (BUILD_JAVA_LANGUAGE_PACKAGE)
	# Java (Latest LTS version):
	find_package(Java 21 REQUIRED)
	message(STATUS "Found Java ${Java_VERSION}: ${Java_JAVAC_EXECUTABLE}")

	# JNI:
	find_package(JNI REQUIRED)
	message(STATUS "Found JNI ${JNI_VERSION}: ${JNI_INCLUDE_DIRS}")

	# Maven:
	find_program(MVN_COMMAND NAMES "mvn" REQUIRED)
	message(STATUS "Found Maven: ${MVN_COMMAND}")

	add_subdirectory(src/lib_java)
	add_subdirectory(java_indexer)
endif()

###############################################################################
# Sourcetrail_indexer
###############################################################################

add_subdirectory(src/indexer)

###############################################################################
# App / Sourcetrail
###############################################################################

function(copyAppFiles configDir)
	copyFile("src/app/Sourcetrail.ico" "${configDir}/app/Sourcetrail.ico")
	if (UNIX)
		copyFile("scripts/Sourcetrail.sh"      "${configDir}/app/Sourcetrail.sh")
		copyFile("scripts/resetPreferences.sh" "${configDir}/app/resetPreferences.sh")
		copyFile("scripts/_gdbinit"            "${configDir}/app/.gdbinit")
	endif()
endfunction()
forEachBuildConfiguration(copyAppFiles)

add_subdirectory(src/app)

###############################################################################
# Sourcetrail_test
###############################################################################

if (BUILD_UNIT_TESTS_PACKAGE)
	set(CATCH2_MIN_VERSION 3.7)
	set(CATCH2_MAX_VERSION 3.16)

	find_package(Catch2 CONFIG REQUIRED)
	message(STATUS "Found Catch2 ${Catch2_VERSION}")
	checkVersionRange("Catch2" ${Catch2_VERSION} ${CATCH2_MIN_VERSION} ${CATCH2_MAX_VERSION})
	include(Catch)

	add_subdirectory(src/test)
endif()

# Create license information:
include(cmake/licenses.cmake)
configureLicenseFile(src/lib_gui/licenses.h)

# Enable this code to find out *all* imported targets:
# get_property(importedTargets DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY IMPORTED_TARGETS)
# message(STATUS "Imported targets: ${importedTargets}")

###############################################################################
# Installing/Packaging
###############################################################################

install(DIRECTORY "${CMAKE_BINARY_DIR}/app/" DESTINATION "${PROJECT_NAME}/app/" USE_SOURCE_PERMISSIONS)
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)

set(CPACK_PACKAGE_VERSION ${CMAKE_PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Source code explorer")
set(CPACK_PACKAGE_DESCRIPTION "Free and open-source cross-platform source explorer that helps you get productive on unfamiliar source code.")

set(CPACK_PACKAGE_CONTACT "P. Most <pmost@pera-software.com>")
set(CPACK_PACKAGE_VENDOR "PERA Software Solutions GmbH")
set(CPACK_PACKAGE_HOMEPAGE_URL ${PROJECT_HOMEPAGE_URL})
set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/src/app/Sourcetrail.ico")

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
