#include "MavenPathDetectorUnixWindows.h"

#include "FilePath.h"
#include "utilityApp.h"

MavenPathDetectorUnixWindows::MavenPathDetectorUnixWindows(const std::string &name)
	: PathDetector(name)
{}

std::vector<FilePath> MavenPathDetectorUnixWindows::doGetPaths() const
{
	std::vector<FilePath> paths;

	bool ok;
	FilePath mavenPath(utility::searchPath("mvn", ok));
	if (ok && !mavenPath.empty() && mavenPath.exists())
	{
		paths.push_back(mavenPath);
	}
	return paths;
}
