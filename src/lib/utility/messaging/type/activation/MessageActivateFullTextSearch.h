#ifndef MESSAGE_ACTIVATE_FULLTEXT_SEARCH_H
#define MESSAGE_ACTIVATE_FULLTEXT_SEARCH_H

#include "Message.h"
#include "MessageActivateBase.h"
#include "TabIds.h"

class MessageActivateFullTextSearch
	: public Message<MessageActivateFullTextSearch>
	, public MessageActivateBase
{
public:
	static const std::string getStaticType()
	{
		return "MessageActivateFullTextSearch";
	}

	MessageActivateFullTextSearch(const std::string& searchTerm, bool caseSensitive = false)
		: searchTerm(searchTerm), caseSensitive(caseSensitive)
	{
		setSchedulerId(TabIds::currentTab());
	}

	void print(std::ostream& os) const override
	{
		os << searchTerm;
	}

	std::vector<SearchMatch> getSearchMatches() const override
	{
		std::string prefix(caseSensitive ? 2 : 1, SearchMatch::FULLTEXT_SEARCH_CHARACTER);
		SearchMatch match(prefix + searchTerm);
		match.searchType = SearchMatch::SEARCH_FULLTEXT;
		return {match};
	}

	const std::string searchTerm;
	bool caseSensitive;
};

#endif	  // MESSAGE_ACTIVATE_FULLTEXT_SEARCH_H
