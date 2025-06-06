#ifndef QT_CODE_NAVIGABLE_H
#define QT_CODE_NAVIGABLE_H

#include <set>

#include "CodeFocusHandler.h"
#include "CodeScrollParams.h"
#include "CodeSnippetParams.h"
#include "types.h"

class FilePath;
class QRectF;
class QAbstractScrollArea;
class QRect;
class QtCodeArea;
class QWidget;

class QtCodeNavigable
{
public:
	virtual ~QtCodeNavigable();

	virtual QAbstractScrollArea* getScrollArea() = 0;

	virtual void updateSourceLocations(const CodeSnippetParams& params) = 0;
	virtual void updateFiles() = 0;

	virtual void scrollTo(
		const FilePath& filePath,
		size_t lineNumber,
		Id locationId,
		Id scopeLocationId,
		bool animated,
		CodeScrollParams::Target target,
		bool focusTarget) = 0;

	virtual void onWindowFocus() = 0;

	virtual void findScreenMatches(
		const std::string& query, std::vector<std::pair<QtCodeArea*, Id>>* screenMatches) = 0;

	virtual void setFocus(Id locationId) = 0;
	virtual void setFocusOnTop() = 0;
	virtual void moveFocus(
		const CodeFocusHandler::Focus& focus, CodeFocusHandler::Direction direction) = 0;

	virtual void copySelection() = 0;

protected:
	void ensureWidgetVisibleAnimated(
		const QWidget* parentWidget,
		const QWidget* childWidget,
		const QRectF& rect,
		bool animated,
		CodeScrollParams::Target target);
	void ensurePercentVisibleAnimated(
		double percentA, double percentB, bool animated, CodeScrollParams::Target target);

	static QRect getFocusRectForWidget(const QWidget* childWidget, const QWidget* parentWidget);
};

#endif	  // QT_CODE_NAVIGABLE_H
