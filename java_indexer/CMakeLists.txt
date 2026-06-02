#
# Maven build (java-indexer.jar)
#
# In case a warning about native access is appearing. Save this content to './.mvn/jvm.config':
# --enable-native-access=ALL-UNNAMED

if (NOT CMAKE_VERBOSE_MAKEFILE)
	set(quietSwitch "--quiet")
endif()

if (NOT BUILD_UNIT_TESTS_PACKAGE)
	set(skipTestsSwitch "-Dmaven.test.skip=true")
endif()

# The Java indexer expects the name 'java-indexer.jar':
set(jarName "java-indexer") # '.jar' extension will be added by maven
makeBuildConfigurationPath("/app/data/java/lib/" jarDirectory)

add_custom_target(Sourcetrail_java_indexer ALL
	# COMMAND ${CMAKE_COMMAND} -E echo "[Maven]: Building ${jarName}.jar to: ${jarDirectory}"
	
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"	
	
	COMMAND "${MVN_COMMAND}" package "-Dsourcetrail.jar.name=${jarName}" "-Dsourcetrail.jar.directory=${jarDirectory}"
		${quietSwitch} ${skipTestsSwitch} 
)
