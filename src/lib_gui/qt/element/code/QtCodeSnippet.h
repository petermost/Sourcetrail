#ifndef QT_CODE_SNIPPET_H
#define QT_CODE_SNIPPET_H

#include <vector>

#include <QFrame>

#include "types.h"

#include "QtCodeArea.h"

#include "CodeFocusHandler.h"
#include "CodeSnippetParams.h"

class QBoxLayout;
class QPushButton;
class QtCodeFile;
class QtCodeNavigator;
class QtHoverButton;
class SourceLocationFile;

class QtCodeSnippet: public QFrame
{
	Q_OBJECT

public:
	QtCodeSnippet(const CodeSnippetParams& params, QtCodeNavigator* navigator, QtCodeFile* file);
	~QtCodeSnippet() override;

	QtCodeFile* getFile() const;
	QtCodeArea* getArea() const;

	size_t getStartLineNumber() const;
	size_t getEndLineNumber() const;

	int lineNumberDigits() const;

	void updateSourceLocations(const CodeSnippetParams& params);

	void updateLineNumberAreaWidthForDigits(int digits);
	void updateContent();

	void setIsActiveFile(bool isActiveFile);

	size_t getLineNumberForLocationId(Id locationId) const;
	QtCodeArea::LineNumbers getLineNumbersForLocationId(Id locationId) const;

	Id getFirstActiveLocationId(Id tokenId) const;
	QRectF getLineRectForLineNumber(size_t lineNumber) const;

	void findScreenMatches(
		const std::string& query, std::vector<std::pair<QtCodeArea*, Id>>* screenMatches);

	bool hasFocus(const CodeFocusHandler::Focus& focus) const;
	bool setFocus(Id locationId);
	bool moveFocus(const CodeFocusHandler::Focus& focus, CodeFocusHandler::Direction direction);
	void focusTop();
	void focusBottom();

	void copySelection();

	void ensureLocationIdVisible(Id locationId, bool animated);

private slots:
	void clickedTitle();
	void clickedFooter();

private:
	QtHoverButton* createScopeLine(QBoxLayout* layout);
	void updateDots();
	void updateScopeLineFocus(QPushButton* line, QPushButton* dots);

	QtCodeNavigator* m_navigator;
	QtCodeFile* m_file;

	Id m_titleId;
	std::string m_titleString;

	Id m_footerId;
	std::string m_footerString;

	std::vector<QPushButton*> m_dots;

	QtHoverButton* m_title = nullptr;
	QtHoverButton* m_footer = nullptr;

	QPushButton* m_titleDots = nullptr;
	QPushButton* m_footerDots = nullptr;

	QtCodeArea* m_codeArea = nullptr;
};

#endif	  // QT_CODE_SNIPPET_H
