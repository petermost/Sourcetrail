# Minimum required version for module support:
cmake_minimum_required(VERSION 3.31) 

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(Usages STATIC)

target_sources(Usages
	PUBLIC		
		FILE_SET
			CXX_MODULES
		
		FILES
			data/module_a_exporter.cppm
			data/module_b_exporter.cppm
	
	PRIVATE
		data/auto_usages.cpp
		data/concept_usages.cpp
		
		data/module_a_exporter.cpp
		data/module_b_exporter.cpp
		data/module_importer.cpp
)
