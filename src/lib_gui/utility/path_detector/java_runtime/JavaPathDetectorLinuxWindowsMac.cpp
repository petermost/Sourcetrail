#include "JavaPathDetectorLinuxWindowsMac.h"

#include "FilePath.h"
#include "utilityApp.h"

#include <QByteArray>
#include <QtEnvironmentVariables>

using namespace std;
using namespace string_literals;
using namespace utility;

namespace
{

FilePath getFilePathRelativeToJavaExecutable(const FilePath& javaExecutablePath)
{
	string relativeJvmLibPath;

	// If those paths change then adjust:
	// - The place holder hints in QtProjectWizardContentPreferences.cpp
	// - The documentation in DOCUMENTATION.md

	if constexpr (Platform::isLinux())
		relativeJvmLibPath = "/../lib/server/libjvm.so"s;
	else if constexpr (Platform::isWindows())
		relativeJvmLibPath = "/./server/jvm.dll"s;
	else if constexpr (Platform::isMac())
		relativeJvmLibPath = "/../lib/server/libjvm.dylib"s;

	FilePath jvmLibPath = javaExecutablePath.getParentDirectory().concatenate(relativeJvmLibPath);
	if (jvmLibPath.exists())
		return jvmLibPath.makeCanonical();

	return FilePath();
}

FilePath getJavaInPath()
{
	bool ok;
	FilePath javaPath(searchPath("java"s, ok));
	if (ok && !javaPath.empty() && javaPath.exists())
	{
		return javaPath;
	}
	return FilePath();
}

FilePath getJavaInJavaHome()
{
	const QByteArray p = qgetenv("JAVA_HOME");
	if (p.isNull())
	{
		return FilePath();
	}

	FilePath javaPath(p);
	javaPath.concatenate(FilePath("/bin/java"s));
	if (!javaPath.empty() && javaPath.exists())
	{
		return javaPath;
	}
	return FilePath();
}

FilePath readLink(const FilePath& path)
{
	return path.getCanonical().getAbsolute();
}

}

JavaPathDetectorLinuxWindowsMac::JavaPathDetectorLinuxWindowsMac(const string &name)
	: JavaPathDetector(name)
{
}

vector<FilePath> JavaPathDetectorLinuxWindowsMac::doGetPaths() const
{
	vector<FilePath> paths;
	FilePath p = getJavaInPath();
	if (!p.empty())
	{
		paths.push_back(p);
	}
	p = getJavaInJavaHome();
	if (!p.empty())
	{
		paths.push_back(p);
	}
	for (const FilePath& path: paths)
	{
		FilePath absoluteJavaPath = readLink(path);
		FilePath jvmLibrary = getFilePathRelativeToJavaExecutable(absoluteJavaPath);
		if (jvmLibrary.exists())
		{
			vector<FilePath> foundPath = {jvmLibrary};
			return foundPath;
		}
	}
	return vector<FilePath>();
}

