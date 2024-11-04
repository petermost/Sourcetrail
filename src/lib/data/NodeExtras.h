#if !defined(NODE_EXTRAS_H)
#define NODE_EXTRAS_H

#include "types.h"

#include <string>
#include <sstream>
#include <map>

struct CustomCommand{
    std::string description;
    std::string command;
};

struct NodeExtras{
    static std::map<Id,std::string> hoverText;
    static std::map<Id,CustomCommand> customCommands;
};

#endif // NODE_EXTRAS_H
