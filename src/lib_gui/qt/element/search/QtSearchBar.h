#ifndef QT_SEARCH_BAR_H
#define QT_SEARCH_BAR_H

#include <string>

#include <QAbstractItemView>
#include <QFrame>

#include "SearchMatch.h"

class QtSearchBarButton;
class QtSmartSearchBox;

class QtSearchBar: public QFrame
{
	Q_OBJECT

public:
	QtSearchBar();

	QSize sizeHint() const override;

	QString query() const;

	void setMatches(const std::vector<SearchMatch>& matches);
	void setFocus();
	void findFulltext();
	void setAutocompletionList(const std::vector<SearchMatch>& autocompletionList);

	QAbstractItemView* getCompleterPopup();

	void refreshStyle();

private slots:
	static void homeButtonClicked();

	static void requestAutocomplete(const std::string& query, NodeTypeSet acceptedNodeTypes);
	static void requestSearch(const std::vector<SearchMatch>& matches, NodeTypeSet acceptedNodeTypes);
	static void requestFullTextSearch(const std::string& query, bool caseSensitive);

private:
	QWidget* m_searchBoxContainer;	  // used for correct clipping inside the search box

	QtSmartSearchBox* m_searchBox;

	QtSearchBarButton* m_searchButton;
	QtSearchBarButton* m_homeButton;
};

#endif	  // QT_SEARCH_BAR_H
