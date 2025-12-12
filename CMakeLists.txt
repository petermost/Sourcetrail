cmake_minimum_required(VERSION 3.27)

include(CMakePrintHelpers)
include(cmake/Sourcetrail.cmake)

# Detect vcpkg build scenario here, so we don't depend on CMakePresets settings:
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

# Make the build type available as a compiler variable:
if (isVcpkgBuild)
	add_compile_definitions(IS_VCPKG_BUILD=1)
else()
	add_compile_definitions(IS_VCPKG_BUILD=0)
endif()

# Project ----------------------------------------------------------------------

project(Sourcetrail VERSION 2025.12.12 HOMEPAGE_URL "http://sourcetrail.de")
cmake_print_variables(CMAKE_C_COMPILER)
cmake_print_variables(CMAKE_CXX_COMPILER)

# Configure which language(s)/package(s) should be supported:
option(BUILD_CXX_LANGUAGE_PACKAGE  "Add C/C++ support to the Sourcetrail indexer." OFF)
option(BUILD_JAVA_LANGUAGE_PACKAGE "Add Java support to the Sourcetrail indexer." OFF)
option(BUILD_UNIT_TESTS_PACKAGE    "Build the corresponding language unit tests." OFF)

configure_file(cmake/language_packages.h.in src/lib/language_packages.h)


# Make Ninja build verbose as well (https://github.com/ninja-build/ninja/issues/900):
if (CMAKE_VERBOSE_MAKEFILE)
	set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "Verbose Makefile" FORCE)
endif()

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

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

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_FIND_PACKAGE_SORT_ORDER NATURAL)

# Prepare Data ----------------------------------------------------------------

if (UNIX)
	configure_file(scripts/generate_coverage_report.sh generate_coverage_report.sh COPYONLY)
endif()

get_property(isMultiConfigGenerator GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if (isMultiConfigGenerator)
	message(FATAL_ERROR "Multi-configuration generators are not supported!")
	# TODO: Test/Support multi-configutration generators
	# foreach(CONFIGURATION_TYPE ${CMAKE_CONFIGURATION_TYPES})
	# 	file(COPY "bin/app/data/"  DESTINATION "${CONFIGURATION_TYPE}/app/data/")
	# 	file(COPY "bin/app/user/"  DESTINATION "${CONFIGURATION_TYPE}/app/user/")
	# 	file(COPY "bin/test/data/" DESTINATION "${CONFIGURATION_TYPE}/test/data/")
	# endforeach()
else()
	set(appDataSourceDir "${CMAKE_SOURCE_DIR}/bin/app/")
	set(appDataTargetDir "${CMAKE_BINARY_DIR}/app/")
	
	message(STATUS "Copying: ${appDataSourceDir} -> ${appDataTargetDir}")
	file(COPY "${appDataSourceDir}" DESTINATION "${appDataTargetDir}")

	if (BUILD_UNIT_TESTS_PACKAGE)
		# The test data contains symlinked files and directories which are used from 'FilePathTestSuite' 
		# and 'FileSystemTestSuite'. CMake, as of this writing (2025-03-07), doesn't copy symlinks 
		# correctly (https://gitlab.kitware.com/cmake/cmake/-/issues/14609) or rather it depends on the 
		# underlying platform.
		
		set(testDataSourceDir "${CMAKE_SOURCE_DIR}/bin/test/data/")
		set(testDataTargetParentDir "${CMAKE_BINARY_DIR}/test/")
		set(testDataTargetDir "${testDataTargetParentDir}data") # Without a '/' at the end!
		
		if (UNIX)
			# Under Linux the symlinks are copied correctly, so copy the data:
			message(STATUS "Copying: ${testDataSourceDir} -> ${testDataTargetDir}")
			file(COPY "${testDataSourceDir}" DESTINATION "${testDataTargetDir}")
		else()
			# Under Windows the symlinks are *not* copied correctly (results in invalid files), so 
			# symlink the data.
			# Note: Temporary test files will show up in version control!
			message(STATUS "Linking: ${testDataSourceDir} -> ${testDataTargetDir}")
			file(MAKE_DIRECTORY "${testDataTargetParentDir}")
			file(CREATE_LINK "${testDataSourceDir}" "${testDataTargetDir}" SYMBOLIC)
		endif()
	endif()
endif()

# ICU --------------------------------------------------------------------------

find_package(ICU REQUIRED 
	COMPONENTS 
		i18n uc data
)
message(STATUS "Found ICU ${ICU_VERSION}: ${ICU_INCLUDE_DIRS}, ${ICU_LIBRARIES}")

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
		
# Boost ------------------------------------------------------------------------

set(Boost_NO_WARN_NEW_VERSIONS ON)

if (isVcpkgBuild)
	# Associated bug entry: https://gitlab.kitware.com/cmake/cmake/-/issues/21200 has been closed.
	set(Boost_NO_SYSTEM_PATHS ON)
endif()

set(BOOST_MIN_VERSION 1.83)
set(BOOST_MAX_VERSION 1.90)

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

# Qt ---------------------------------------------------------------------------

set(QT_NO_PRIVATE_MODULE_WARNING ON)

set(QT_MIN_VERSION 6.8.3)
# set (QT_MAX_VERSION_HEX 0x060802)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED 
	COMPONENTS
		Network 
		# PrintSupport 
		Svg
		Widgets 
)
message(STATUS "Found Qt ${Qt6_VERSION}")

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
		Qt6::Network
		# Qt6::PrintSupport
		Qt6::Svg
		Qt6::Widgets
)

# SQLite3 ----------------------------------------------------------------------

if (isVcpkgBuild)
	find_package(unofficial-sqlite3 CONFIG REQUIRED)
	if (NOT DEFINED SQLite3_VERSION)
		# Use quotes to indicate a version literal:
		set(SQLite3_VERSION "\"3.50.4\"")
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
message(STATUS "Found tinyxml ${tinyxml_VERSION}")

# AidKit  -----------------------------------------------------------------------------------

add_subdirectory(src/lib_aidkit)

# External Lib CppSQlite3  ------------------------------------------------------------------

add_subdirectory(src/external)
# message(STATUS "Using CppSQLite3 ${CppSQLite3_VERSION}")

# Lib Gui ----------------------------------------------------------------------

configure_file(cmake/productVersion.h.in src/lib_gui/productVersion.h)

add_subdirectory(src/resources)
add_subdirectory(src/lib_gui)

# Lib --------------------------------------------------------------------------

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

# Lib Cxx ----------------------------------------------------------------------

if (BUILD_CXX_LANGUAGE_PACKAGE)

	# Integrate LLVM/LibTooling. 
	# For further details see https://llvm.org/docs/CMake.html#embedding-llvm-in-your-project

	set(CLANG_MIN_VERSION 18)
	set(CLANG_MAX_VERSION 21)
	
	find_package(Clang CONFIG REQUIRED) # Minimum version and/or version range xx...<yy doesn't work.
	message(STATUS "Found LLVM ${LLVM_VERSION}: ${LLVM_INCLUDE_DIRS}")
	checkVersionRange("Clang" ${LLVM_VERSION} ${CLANG_MIN_VERSION} ${CLANG_MAX_VERSION})

	add_library(External_lib_clang INTERFACE)
	
	separate_arguments(llvmDefinitions NATIVE_COMMAND ${LLVM_DEFINITIONS})
	target_compile_definitions(External_lib_clang
		INTERFACE
			${llvmDefinitions}
	)
	target_include_directories(External_lib_clang SYSTEM
		INTERFACE
			${LLVM_INCLUDE_DIRS}
			${CLANG_INCLUDE_DIRS}
	)
	
	# 'LLVM_LINK_LLVM_DYLIB' is set when LLVM/Clang have been build into seperate single libraries.
	# Apparently this means for LLVM 'libLLVM' and for Clang 'libclang-cpp'.
	# For further details see https://llvm.org/docs/CMake.html#embedding-llvm-in-your-project
	
	if (LLVM_LINK_LLVM_DYLIB) # Should be CLANG_LINK_CLANG_DYLIB in future LLVM releases?
		set(llvmLibraries LLVM)
		set(clangLibraries clang-cpp)
	else()
		llvm_map_components_to_libnames(llvmLibraries
			${LLVM_TARGETS_TO_BUILD} support core libdriver passes option
		)
		# LibTooling libraries:
		set(clangLibraries
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
		
	target_link_libraries(External_lib_clang
		INTERFACE
			${llvmLibraries}
			${clangLibraries}
	)
	
	# Copy required headers:
	if (isVcpkgBuild)
		set(headerSourceDir "${LLVM_TOOLS_BINARY_DIR}/lib/clang/${LLVM_VERSION_MAJOR}/include/")
	else()
		set(headerSourceDir "${LLVM_TOOLS_BINARY_DIR}/../lib/clang/${LLVM_VERSION_MAJOR}/include/")
	endif()
	set(headerTargetDir "${CMAKE_BINARY_DIR}/app/data/cxx/include/")

	message(STATUS "Copying LLVM header: ${headerSourceDir} -> ${headerTargetDir}")
	file(COPY "${headerSourceDir}" DESTINATION "${headerTargetDir}")
	
	add_subdirectory(src/lib_cxx)
else()
	message(STATUS "Building the C++ indexer is disabled. You can enable it by setting 'BUILD_CXX_LANGUAGE_PACKAGE' to 'ON'.")
endif()

# Lib Java ---------------------------------------------------------------------

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
else()
	message(STATUS "Building the Java indexer is disabled. You can enable it by setting 'BUILD_JAVA_LANGUAGE_PACKAGE' to 'ON'.")
endif()

# Indexer App ------------------------------------------------------------------

add_subdirectory(src/indexer)

# App --------------------------------------------------------------------------

configure_file(src/app/Sourcetrail.ico app/Sourcetrail.ico COPYONLY)
if (UNIX)
	configure_file(scripts/Sourcetrail.sh  app/Sourcetrail.sh COPYONLY)
	configure_file(scripts/_gdbinit        app/.gdbinit COPYONLY)
endif()
add_subdirectory(src/app)

# Test ----------------------------------------------------------------------

set(CATCH2_MIN_VERSION 3.7)
set(CATCH2_MAX_VERSION 3.12)

if (BUILD_UNIT_TESTS_PACKAGE)
	include(CTest)

	find_package(GTest CONFIG REQUIRED)
	message(STATUS "Found GTest ${GTest_VERSION}")
	include(GoogleTest)
	add_subdirectory(src/test_aidkit)

	find_package(Catch2 CONFIG REQUIRED)
	message(STATUS "Found Catch2 ${Catch2_VERSION}")
	checkVersionRange("Catch2" ${Catch2_VERSION} ${CATCH2_MIN_VERSION} ${CATCH2_MAX_VERSION})
	include(Catch)
	add_subdirectory(src/test)
else()
	message(STATUS "Building the tests is disabled. You can enable them by setting 'BUILD_UNIT_TESTS_PACKAGE' to 'ON'.")
endif()

# Create license information:
include(cmake/licenses.cmake)
configureLicenseFile(src/lib_gui/licenses.h)

# Some sanity checks:
# Note: We doing these checks last so the messages are at the end of a possible long output from cmake.

if (NOT BUILD_CXX_LANGUAGE_PACKAGE AND NOT BUILD_JAVA_LANGUAGE_PACKAGE)
	message(WARNING "Building ONLY the GUI, because all language indexers are disabled!")
endif()

# prohibit in-source-builds
if (${CMAKE_BINARY_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
	message(FATAL_ERROR "In-source-builds are not supported!")
endif()

if (BUILD_PYTHON_LANGUAGE_PACKAGE)
	message(FATAL_ERROR "Python support has been removed!")
	# See:
	# - https://github.com/petermost/Sourcetrail/issues/34
	# - https://github.com/CoatiSoftware/SourcetrailPythonIndexer
endif()

# Enable this code to find out *all* imported targets:
# get_property(importedTargets DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY IMPORTED_TARGETS)
# message(STATUS "Imported targets: ${importedTargets}")

# Installing/Packaging ---------------------------------------------------------

install(DIRECTORY "${CMAKE_BINARY_DIR}/app/" DESTINATION "${PROJECT_NAME}/app/" USE_SOURCE_PERMISSIONS)
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)

set(CPACK_PACKAGE_VERSION ${CMAKE_PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Source code explorer")
set(CPACK_PACKAGE_DESCRIPTION "Free and open-source cross-platform source explorer that helps you get productive on unfamiliar source code.")

set(CPACK_PACKAGE_CONTACT "P. Most <pmost@pera-software.com>")
set(CPACK_PACKAGE_VENDOR "PERA Software Solutions GmbH")
set(CPACK_PACKAGE_HOMEPAGE_URL ${PROJECT_HOMEPAGE_URL})
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
