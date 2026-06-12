add_executable(Sourcetrail WIN32
	main.cpp
	Sourcetrail.rc
)
setDefaultTargetOptions(Sourcetrail)

setAppOutputDirectory(Sourcetrail)

target_link_libraries(Sourcetrail
	PRIVATE
		Sourcetrail_lib
)
