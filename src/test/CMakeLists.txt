add_executable(Sourcetrail_test
	helper/TestFileRegister.cpp
	helper/TestFileRegister.h
	helper/TestStorage.h
	helper/TestStorage.cpp

	Catch2.hpp
	Catch2.cpp
	test_main.cpp

	CommandlineTestSuite.cpp
	CompilationDatabaseTestSuite.cpp
	ConfigManagerTestSuite.cpp
	CxxIncludeProcessingTestSuite.cpp
	CxxParserTestSuite.cpp
	CxxTypeNameTestSuite.cpp
	FileManagerTestSuite.cpp
	FilePathFilterTestSuite.cpp
	FilePathTestSuite.cpp
	FileSystemTestSuite.cpp
	GraphTestSuite.cpp
	HierarchyCacheTestSuite.cpp
	JavaIndexSampleProjectsTestSuite.cpp
	JavaParserTestSuite.cpp
	LogManagerTestSuite.cpp
	LowMemoryStringMapTestSuite.cpp
	MatrixBaseTestSuite.cpp
	MatrixDynamicBaseTestSuite.cpp
	MessageQueueTestSuite.cpp
	NetworkProtocolHelperTestSuite.cpp
	PlatformTestSuite.cpp
	PythonIndexerTestSuite.cpp
	QtResourcesTestSuite.cpp
	RefreshInfoGeneratorTestSuite.cpp
	SearchIndexTestSuite.cpp
	SettingsMigratorTestSuite.cpp
	SettingsTestSuite.cpp
	SharedMemoryTestSuite.cpp
	SourceGroupTestSuite.cpp
	SourceLocationCollectionTestSuite.cpp
	SqliteBookmarkStorageTestSuite.cpp
	SqliteIndexStorageTestSuite.cpp
	StorageTestSuite.cpp
	TaskSchedulerTestSuite.cpp
	TextAccessTestSuite.cpp
	TextCodecTestSuite.cpp
	UtilityGradleTestSuite.cpp
	UtilityMavenTestSuite.cpp
	UtilityStringTestSuite.cpp
	UtilityEnumTestSuite.cpp
	Vector2TestSuite.cpp
	Snippet.cpp
)
setDefaultTargetOptions(Sourcetrail_test)

target_include_directories(Sourcetrail_test
	PRIVATE
		helper
)

set_target_properties(Sourcetrail_test
	PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test/"
		PDB_OUTPUT_DIRECTORY     "${CMAKE_BINARY_DIR}/test/"
)

target_link_libraries(Sourcetrail_test
	PRIVATE
		Sourcetrail_lib
		Sourcetrail_res_gui
		Catch2::Catch2
)

if (WIN32)
	set_target_properties(Sourcetrail_test
		PROPERTIES
			VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/test"
	)
endif ()

catch_discover_tests(Sourcetrail_test)
