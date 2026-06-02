add_executable(Sourcetrail_indexer
	main.cpp
)
setDefaultTargetOptions(Sourcetrail_indexer)

setAppOutputDirectory(Sourcetrail_indexer)

set_target_properties(Sourcetrail_indexer
	PROPERTIES
		OUTPUT_NAME sourcetrail_indexer
)

target_link_libraries(Sourcetrail_indexer
	PRIVATE
		Sourcetrail_lib
)
