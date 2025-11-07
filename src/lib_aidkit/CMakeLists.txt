add_library(AidKit_lib
	aidkit/enum_class.cpp

	aidkit/shared_data.cpp
	
	aidkit/qt/Strings.hpp
	aidkit/qt/Strings.cpp
)

target_sources(AidKit_lib
	PUBLIC		
		FILE_SET
			CXX_MODULES
		
		FILES
			aidkit/enum_class.cppm
			aidkit/shared_data.cppm
)

setDefaultTargetOptions(AidKit_lib)

target_include_directories(AidKit_lib
	PUBLIC
		.
)
target_link_libraries(AidKit_lib
	PUBLIC
		External_lib_qt
)

