#ifndef JAVA_ENVIRONMENT_FACTORY_H
#define JAVA_ENVIRONMENT_FACTORY_H

#include <aidkit/thread_shared.hpp>

#include <jni.h>

#include <boost/dll/shared_library.hpp>

#include <map>
#include <memory>
#include <string>
#include <thread>

class JavaEnvironment;

class JavaEnvironmentFactory
{
public:
	static void createInstance(const std::string &classPath, std::string *errorString);
	static std::shared_ptr<JavaEnvironmentFactory> getInstance();

	~JavaEnvironmentFactory();

	std::shared_ptr<JavaEnvironment> createEnvironment();

private:
	friend class JavaEnvironment;

	static boost::dll::shared_library s_jvmLibrary;
	static std::shared_ptr<JavaEnvironmentFactory> s_instance;
	static std::string s_classPath;

	JavaEnvironmentFactory(JavaVM* jvm);

	void registerEnvironment();
	void unregisterEnvironment();

	JavaVM *const m_jvm;
	aidkit::thread_shared<std::map<std::thread::id, std::pair<JNIEnv*, int /* reference counter */>>> m_threadIdToEnvAndUserCount;
};

#endif	  // JAVA_ENVIRONMENT_FACTORY_H
