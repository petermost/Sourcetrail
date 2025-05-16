#ifndef QT_SMART_SEARCH_BOX_H
#define QT_SMART_SEARCH_BOX_H

#include <deque>

#include <QLineEdit>
#include <QPushButton>

#include "QtAutocompletionList.h"
#include "SearchMatch.h"

class NodeTypeSet;

class QtSearchElement: public QPushButton
{
	Q_OBJECT

signals:
	void wasChecked(QtSearchElement*);

public:
	QtSearchElement(const QString& text, QWidget* parent);

private slots:
	void onChecked(bool);
};


class QtSmartSearchBox: public QLineEdit
{
	Q_OBJECT
	
public:
	explicit QtSmartSearchBox(QWidget* parent = nullptr);

	// Shadow QLineEdit::setPlaceholderText:
	void setPlaceholderText(const QString &);
	void setFullTextSearchSupport(bool supportsFullTextSearch);
	
	QCompleter* getCompleter() const;
	std::vector<SearchMatch> getMatches() const;
	
	void setAutocompletionList(const std::vector<SearchMatch>& autocompletionList);
	void setMatches(const std::vector<SearchMatch>& matches);
	void setFocus();
	void findFulltext();
	
	void refreshStyle();
	
signals:
	void autocomplete(const std::string& query, NodeTypeSet acceptedNodeTypes);
	void search(const std::vector<SearchMatch>& matches, NodeTypeSet acceptedNodeTypes);
	void fullTextSearch(const std::string& query, bool caseSensitive);
	
public slots:
	void startSearch();

protected:
	bool event(QEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;

	void focusInEvent(QFocusEvent* event) override;
	void focusOutEvent(QFocusEvent* event) override;

	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;

	void mouseMoveEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

private slots:
	void onTextEdited(const QString& text);
	void onTextChanged(const QString& text);

	void onAutocompletionHighlighted(const SearchMatch& match);
	void onAutocompletionActivated(const SearchMatch& match);

	void onElementSelected(QtSearchElement* element);

private:
	void moveCursor(int offset);
	void moveCursorTo(int goal);

	void addMatch(const SearchMatch& match);
	void addMatchAndUpdate(const SearchMatch& match);
	void clearMatches();

	void setEditText(const QString& text);
	bool editTextToElement();
	SearchMatch editElement(QtSearchElement* element);

	void updateElements();
	void layoutElements();

	bool hasSelectedElements() const;
	std::string getSelectedString() const;

	void selectAllElementsWith(bool selected);
	void selectElementsTo(size_t idx, bool selected);
	void deleteSelectedElements();

	void updatePlaceholder();
	void clearLineEdit();

	void requestAutoCompletions();
	void hideAutoCompletions();

	void startFullTextSearch();

	static std::deque<SearchMatch> getMatchesForInput(const std::string& text);

	NodeTypeSet getMatchAcceptedNodeTypes() const;
	bool lastMatchIsNoFilter() const;

	QString m_placeholder;
	bool m_supportsFullTextSearch = false;

	bool m_allowTextChange = false;
	QString m_oldText;

	std::deque<SearchMatch> m_matches;
	SearchMatch m_oldMatch;

	std::vector<QtSearchElement*> m_elements;

	size_t m_cursorIndex = 0;

	SearchMatch m_highlightedMatch;

	bool m_shiftKeyDown = false;
	bool m_mousePressed = false;
	int m_mouseX = 0;
	bool m_ignoreNextMousePress = false;

	QWidget* m_highlightRect;
	QtAutocompletionList* m_completer;

	int m_oldLayoutOffset = 0;
};

#endif	  // QT_SMART_SEARCH_BOX_H
