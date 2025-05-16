#ifndef UTILITY_APP_H
#define UTILITY_APP_H

#include <string>

#include <boost/chrono/duration.hpp>

#include "FilePath.h"
#include <Platform.h>

namespace utility
{
constexpr boost::chrono::milliseconds DEFAULT_TIMEOUT(30000);
constexpr boost::chrono::milliseconds INFINITE_TIMEOUT(boost::chrono::milliseconds::max());

struct ProcessOutput
{
	std::string output;
	std::string error;
	int exitCode;
};

std::string getDocumentationLink();

std::string searchPath(const std::string& bin, bool& ok);

std::string searchPath(const std::string& bin);

ProcessOutput executeProcess(
	const std::string& command,
	const std::vector<std::string>& arguments,
	const FilePath& workingDirectory = FilePath(),
	const bool waitUntilNoOutput = false,
	const boost::chrono::milliseconds &timeout = DEFAULT_TIMEOUT,
	bool logProcessOutput = false);

void killRunningProcesses();

int getIdealThreadCount();

}

#endif	  // UTILITY_APP_H
