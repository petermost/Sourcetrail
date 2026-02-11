#
# Maven build (java-indexer.jar)
#

# The Java indexer expects the name 'java-indexer.jar':
set(jarName "java-indexer") # '.jar' extension will be added by maven
set(jarDirectory "${CMAKE_BINARY_DIR}/app/data/java/lib/")

if (NOT CMAKE_VERBOSE_MAKEFILE)
	set(quietSwitch "--quiet")
endif()

if (NOT BUILD_UNIT_TESTS_PACKAGE)
	set(skipTestsSwitch "-Dmaven.test.skip=true")
endif()

# In case a warning about native access is appearing. Save this content to './.mvn/jvm.config':
# --enable-native-access=ALL-UNNAMED

add_custom_target(Sourcetrail_java_indexer ALL
	COMMAND ${CMAKE_COMMAND} -E echo "[Maven]: Building ${jarName}.jar to: ${jarDirectory}"
	
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"	
	
	COMMAND "${MVN_COMMAND}" package "-Dsourcetrail.jar.name=${jarName}" "-Dsourcetrail.jar.directory=${jarDirectory}"
		${quietSwitch} ${skipTestsSwitch} 
)
