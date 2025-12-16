#ifndef UTILITY_MAIN_H
#define UTILITY_MAIN_H

#include <string>

// Functions for handling multiple main functions:

class NameHierarchy;

bool isMainFunction(const NameHierarchy &nameHierarchy);
void uniquifyMainFunction(NameHierarchy *main, const std::string &uniqueAppendix);
bool isUniquifiedMainFunction(const NameHierarchy &nameHierarchy);
void deuniquifyMainFunction(NameHierarchy *main);

struct SearchMatch;

bool isMainFunction(const SearchMatch &match);

#endif
