#ifndef SEARCH_VIEW_H
#define SEARCH_VIEW_H

#include "SearchMatch.h"
#include "View.h"

class SearchController;

class SearchView: public View
{
public:
	SearchView(ViewLayout* viewLayout);
	~SearchView() override;

	std::string getName() const override;

	virtual std::string getQuery() const = 0;

	virtual void setMatches(const std::vector<SearchMatch>& matches) = 0;

	virtual void setFocus() = 0;
	virtual void findFulltext() = 0;

	virtual void setAutocompletionList(const std::vector<SearchMatch>& autocompletionList) = 0;

protected:
	SearchController* getController();
};

#endif	  // SEARCH_VIEW_H