#ifndef UTILITY_CXX_HEADER_DETECTION_H
#define UTILITY_CXX_HEADER_DETECTION_H

#include <string>
#include <vector>

#include "Platform.h"
#include "FilePath.h"

namespace utility
{
std::vector<std::string> getCxxHeaderPaths(const std::string& compilerName);

std::vector<FilePath> getWindowsSdkHeaderSearchPaths(Platform::Architecture architecture);
}

#endif	  // UTILITY_CXX_HEADER_DETECTION_H
