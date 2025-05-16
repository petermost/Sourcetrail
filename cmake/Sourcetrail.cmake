function(checkVersionRange libraryName version versionMin versionMax)
	# Version range in find_package has quite often not the desired effect, so check version ranges manually:
	if (NOT (${version} VERSION_GREATER_EQUAL ${versionMin} AND ${version} VERSION_LESS ${versionMax}))
		message(FATAL_ERROR "${libraryName} version ${version} is not supported! Must be in range >= ${versionMin} and < ${versionMax}")
	endif()
endfunction()

function(setGccTargetOptions targetName)
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
			-Wcast-align=strict
			-Wcast-qual
			-Wduplicated-branches
			-Wduplicated-cond
			-Wextra-semi
			# -Wfloat-equal
			# -Wformat=2
			# -Wformat-nonliteral
			-Wformat-overflow=2
			-Wformat-truncation=2
			-Wlogical-op
			-Wnon-virtual-dtor
			# -Wold-style-cast
			# -Woverloaded-virtual
			# -Wundef
			-Wuninitialized
			-Winit-self
			# -Wunused-macros
			# -Wuseless-cast
			-Wwrite-strings

			#
			# Warnings which should be errors:
			#
			# Get the same behaviour as msvc for 'narrowing conversions'
			# See https://gcc.gnu.org/wiki/FAQ#Wnarrowing for further information.
			-Werror=narrowing
			-Werror=suggest-override
			# GCC doesn't seem to have a warning flag 'inconsistent-missing-override' like Clang
			# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=84695

			#
			# Disabled warnings:
			#
			-Wno-comment
			-Wno-implicit-fallthrough
			-Wno-missing-field-initializers
			-Wno-stringop-truncation
			-Wno-unknown-pragmas
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

function(setClangTargetOptions targetName)
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
			# Warnings which should be errors:
			#
			-Werror=c++11-narrowing
			-Werror=suggest-override
			-Werror=inconsistent-missing-override

			#
			# Disabled warnings:
			#
			-Wno-missing-field-initializers
			-Wno-overloaded-virtual
			-Wno-unknown-pragmas
			-Wno-unused-lambda-capture
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
	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		setGccTargetOptions(${targetName})
	elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		setClangTargetOptions(${targetName})
	elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
		setMsvcTargetOptions(${targetName})
	endif()
endfunction()
