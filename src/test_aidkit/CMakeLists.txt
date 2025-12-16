add_executable(AidKit_test
	enum_class_test.cpp
	thread_shared_test.cpp
	
	qt/StringsTest.cpp
)
setDefaultTargetOptions(AidKit_test)

set_target_properties(AidKit_test
	PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test/"
		PDB_OUTPUT_DIRECTORY     "${CMAKE_BINARY_DIR}/test/"
)

target_link_libraries(AidKit_test
	PRIVATE
		AidKit_lib
		GTest::gtest
		GTest::gtest_main
)

if (WIN32)
	set_target_properties(AidKit_test
		PROPERTIES
			VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/test"
	)
endif ()

gtest_discover_tests(AidKit_test)
