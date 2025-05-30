#ifndef COMMANDLINE_HELPER_H
#define COMMANDLINE_HELPER_H

#include "FilePath.h"

#include <string>
#include <vector>

namespace boost::program_options
{
class variables_map;
class options_description;
} // namespace boost::program_options


namespace po = boost::program_options;

namespace commandline
{
void parseConfigFile(po::variables_map& vm, po::options_description& options);

std::vector<FilePath> extractPaths(const std::vector<std::string>& vector);
}	 // namespace commandline

#endif	  // COMMANDLINE_HELPER_H
