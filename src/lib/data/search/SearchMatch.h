#ifndef SEARCH_MATCH_H
#define SEARCH_MATCH_H

#include <ostream>
#include <set>
#include <string>
#include <vector>

#include "Node.h"
#include "types.h"

class NodeTypeSet;

// SearchMatch is used to display the search result in the UI
struct SearchMatch
{
	enum SearchType
	{
		SEARCH_NONE,
		SEARCH_TOKEN,
		SEARCH_COMMAND,
		SEARCH_OPERATOR,
		SEARCH_FULLTEXT
	};

	enum CommandType
	{
		COMMAND_ALL,
		COMMAND_ERROR,
		COMMAND_NODE_FILTER,
		COMMAND_LEGEND
	};

	static void log(const std::vector<SearchMatch>& matches, const std::string& query);

	static std::string getSearchTypeName(SearchType type);
	static std::string searchMatchesToString(const std::vector<SearchMatch>& matches);

	static SearchMatch createCommand(CommandType type);
	static std::vector<SearchMatch> createCommandsForNodeTypes(NodeTypeSet types);
	static std::string getCommandName(CommandType type);

	static const char FULLTEXT_SEARCH_CHARACTER = '?';

	SearchMatch();
	SearchMatch(const std::string& query);

	bool operator<(const SearchMatch& other) const;
	bool operator==(const SearchMatch& other) const;

	static size_t getTextSizeForSorting(const std::string* str);

	bool isValid() const;
	bool isFilterCommand() const;

	void print(std::ostream& ostream) const;

	std::string getFullName() const;
	std::string getSearchTypeName() const;
	CommandType getCommandType() const;

	std::string name;

	std::string text;
	std::string subtext;

	std::vector<Id> tokenIds;
	std::vector<NameHierarchy> tokenNames;

	std::string typeName;

	NodeType nodeType;
	SearchType searchType;
	std::vector<size_t> indices;

	int score = 0;
	bool hasChildren = false;
};


#endif	  // SEARCH_MATCH_H
