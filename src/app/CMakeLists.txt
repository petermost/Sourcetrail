add_executable(Sourcetrail
	main.cpp
	Sourcetrail.rc
)
setDefaultTargetOptions(Sourcetrail)

setAppOutputDirectory(Sourcetrail)

target_link_libraries(Sourcetrail
	PRIVATE
		Sourcetrail_lib
)
