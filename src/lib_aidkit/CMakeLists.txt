add_library(AidKit_lib
	aidkit/enum_class.hpp
	aidkit/enum_class.cpp

	aidkit/shared_data.hpp
	aidkit/shared_data.cpp
	
	aidkit/qt/Strings.hpp
	aidkit/qt/Strings.cpp
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

