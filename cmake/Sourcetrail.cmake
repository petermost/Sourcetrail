if (NOT PROJECT_NAME)
	message(FATAL_ERROR "'Sourcetrail.cmake' must be called after project()!")
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(isGccCompiler TRUE)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	set(isClangCompiler TRUE)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	set(isMsvcCompiler TRUE)
endif()

get_property(isMultiConfigGenerator GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

# Only some commands treat a relative path with respect to the value of CMAKE_CURRENT_SOURCE_DIR|CMAKE_CURRENT_BINARY_DIR
# So always resolve paths with respect to CMAKE_CURRENT_SOURCE_DIR|CMAKE_CURRENT_BINARY_DIR

function(resolveSourcePath pathVar)
	cmake_path(ABSOLUTE_PATH ${pathVar} BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" NORMALIZE)
	set(${pathVar} "${${pathVar}}" PARENT_SCOPE)
endfunction()

function(resolveBinaryPath pathVar)
	cmake_path(ABSOLUTE_PATH ${pathVar} BASE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" NORMALIZE)
	set(${pathVar} "${${pathVar}}" PARENT_SCOPE)
endfunction()

function(forEachBuildConfiguration callbackFunction)
	if(isMultiConfigGenerator)
		foreach(configurationType ${CMAKE_CONFIGURATION_TYPES})
			cmake_language(CALL ${callbackFunction} "${configurationType}")
		endforeach()
	else()
		cmake_language(CALL ${callbackFunction} ".")
	endif()
endfunction()

function(makeBuildConfigurationPath relativeSubPath outputVariable)
	if(isMultiConfigGenerator)
		cmake_path(SET multiConfigPath NORMALIZE "${CMAKE_BINARY_DIR}/$<CONFIG>/${relativeSubPath}")
	else()
		cmake_path(SET multiConfigPath NORMALIZE "${CMAKE_BINARY_DIR}/${relativeSubPath}")
	endif()

	set(${outputVariable} "${multiConfigPath}" PARENT_SCOPE)
endfunction()

function(setTargetOutputDirectory targetName relativeSubPath)
	makeBuildConfigurationPath("${relativeSubPath}" multiConfigPath)

	set_target_properties(${targetName}
		PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY "${multiConfigPath}"
			PDB_OUTPUT_DIRECTORY     "${multiConfigPath}"
	)
endfunction()

function(setAppOutputDirectory targetName)
	setTargetOutputDirectory(${targetName} "/app/")
endfunction()

function(setTestOutputDirectory targetName)
	setTargetOutputDirectory(${targetName} "/test/")
endfunction()

function(configureFile inputFile outputFile)
	resolveSourcePath(inputFile)
	resolveBinaryPath(outputFile)

	message(STATUS "Configuring: ${inputFile} -> ${outputFile}")

	configure_file("${inputFile}" "${outputFile}")
endfunction()

function(copyFile sourceFile targetFile)
	resolveSourcePath(sourceFile)
	resolveBinaryPath(targetFile)

	message(STATUS "Copying: ${sourceFile} -> ${targetFile}")

	file(COPY_FILE "${sourceFile}" "${targetFile}")
endfunction()

function(copyDirectory sourceDirectory targetDirectory)
	resolveSourcePath(sourceDirectory)
	resolveBinaryPath(targetDirectory)

	message(STATUS "Copying: ${sourceDirectory} -> ${targetDirectory}")

	file(COPY "${sourceDirectory}" DESTINATION "${targetDirectory}")
endfunction()

function(symlinkDirectory sourceDirectory targetDirectory)
	resolveSourcePath(sourceDirectory)
	resolveBinaryPath(targetDirectory)
	
	cmake_path(GET targetDirectory PARENT_PATH targetParentDirectory)

	message(STATUS "Linking: ${sourceDirectory} -> ${targetDirectory}")

	file(MAKE_DIRECTORY "${targetParentDirectory}")
	file(CREATE_LINK "${sourceDirectory}" "${targetDirectory}" SYMBOLIC)
endfunction()

function(checkVersionRange libraryName version versionMin versionMax)
	# Version range in find_package has quite often not the desired effect, so check version ranges manually:
	if (NOT (${version} VERSION_GREATER_EQUAL ${versionMin} AND ${version} VERSION_LESS ${versionMax}))
		message(FATAL_ERROR "${libraryName} version ${version} is not supported! Must be in range >= ${versionMin} and < ${versionMax}")
	endif()
endfunction()

function(setCommonGccClangTargetOptions targetName)
	target_compile_options(${targetName}
		PRIVATE
			-finput-charset=UTF-8
			
			-pipe

			# Will lead to a failure in "cxx parser finds braces with closing bracket in macro" !!!
			# -ftrivial-auto-var-init=zero
			
			-Wall
			-Wextra
			-Wpedantic

			#
			# Additional warnings:
			#
			-Wcast-qual
			-Wextra-semi
			-Wnon-virtual-dtor
			-Wwrite-strings
			
			# -Wformat=2 #-Wformat-nonliteral
			
			# GCC: '-Winit-self' is enabled by '-Wall', but '-Wuninitialized' ?
			# Clang: '-Winit-self' is ignored
			-Winit-self -Wuninitialized
			
			#
			# Similar GCC/Clang options:
			#
			$<$<BOOL:${isGccCompiler}>:-Wcast-align=strict>
			$<$<BOOL:${isClangCompiler}>:-Wcast-align>
			
			$<$<BOOL:${isGccCompiler}>:-Wformat-overflow=2>
			$<$<BOOL:${isClangCompiler}>:-Wformat-overflow>
			
			$<$<BOOL:${isGccCompiler}>:-Wformat-truncation=2>
			$<$<BOOL:${isClangCompiler}>:-Wformat-truncation>

			# -Wfloat-equal
			# -Wold-style-cast
			# -Woverloaded-virtual
			# -Wundef
			# -Wunused-macros
			# -Wuseless-cast

			#
			# Warnings which should be errors:
			#
			# Get the same behaviour as msvc for 'narrowing conversions'
			# See https://gcc.gnu.org/wiki/FAQ#Wnarrowing for further information.
			-Werror=narrowing

			# Force `override` usage:
			-Werror=suggest-override
			
			# Should help when extending enums like `SymbolKind`, `NodeKind` etc.:
			-Werror=switch
			-Werror=return-type

			#
			# Disabled warnings:
			#
			-Wno-comment
			-Wno-implicit-fallthrough
			-Wno-missing-field-initializers
			-Wno-unknown-pragmas
			-Wno-overloaded-virtual
	)

	target_compile_definitions(${targetName}
		PRIVATE
			$<$<CONFIG:Debug>:_FORTIFY_SOURCE=3>
			$<$<CONFIG:Debug>:_GLIBCXX_ASSERTIONS>
	
			# We would also like to enable these switches, but then we get linker errors with prebuild
			# libraries like boost, which have been build without them!
			# $<$<CONFIG:Debug>:_GLIBCXX_DEBUG>
			# $<$<CONFIG:Debug>:_GLIBCXX_DEBUG_PEDANTIC>
			# $<$<CONFIG:Debug>:_GLIBCXX_DEBUG_BACKTRACE>
	)
endfunction()

function(setGccTargetOptions targetName)
	setCommonGccClangTargetOptions(${targetName})

	target_compile_options(${targetName}
		PRIVATE
			#
			# Additional warnings:
			#
			-Wduplicated-branches
			-Wduplicated-cond
			-Wlogical-op

			#
			# Disabled warnings:
			#
			-Wno-stringop-truncation
	)

	#[[
	# Add code coverage options:
	target_compile_options(${targetName}
		PRIVATE
			$<$<CONFIG:Debug>:--coverage -fprofile-abs-path>
	)
	target_link_options(${targetName}
		PRIVATE
			$<$<CONFIG:Debug>:--coverage>
	)
	]]
endfunction()

function(setClangTargetOptions targetName)
	setCommonGccClangTargetOptions(${targetName})
	
	target_compile_options(${targetName}
		PRIVATE
			#
			# Disabled warnings:
			#
			-Wno-unused-lambda-capture
	)
endfunction()

function(setMsvcTargetOptions targetName)
	target_compile_options(${targetName}
		PRIVATE
			/utf-8
			
			/nologo
			/MP
			/W4

			#
			# Make msvc standard compliant (sigh):
			#
			/permissive-
			/Zc:__cplusplus
			/Zc:__STDC__
			/Zc:enumTypes
			/Zc:externConstexpr
			/Zc:inline
			/Zc:preprocessor
			/Zc:templateScope
			/Zc:throwingNew

			#
			# Additional warnings:
			#
			/w44265 # class has virtual functions, but destructor is not virtual

			#
			# Warnings which should be errors:
			#
			/we4172 # returning address of local variable or temporary
			/we4840 # non-portable use of class as an argument to a variadic function

			#
			# Disabled warnings:
			#
			/wd4245 # signed/unsigned mismatch
			/wd4250 # inherits via dominance
			/wd4389 # '==': signed/unsigned mismatch
			/wd4456 # declaration hides previous local declaration
			/wd4457 # declaration hides function parameter
			/wd4458 # declaration hides class member
	)
	target_compile_definitions(${targetName}
		PUBLIC
			WIN32_LEAN_AND_MEAN

			UNICODE  # For Windows header
			_UNICODE # For CRT header
			
			# Target Windows 10:
			WINVER=0x0A00
			_WIN32_WINNT=0x0A00

	)
	target_link_options(${targetName}
		PUBLIC
			/NOLOGO
			# The output from the switches bellow are not visible in the output window :-(
			# /verbose:incr
			# /TIME
	)
endfunction()

function(setDefaultTargetOptions targetName)
	if (isGccCompiler)
		setGccTargetOptions(${targetName})
	elseif (isClangCompiler)
		setClangTargetOptions(${targetName})
	elseif (isMsvcCompiler)
		setMsvcTargetOptions(${targetName})
	endif()
endfunction()
