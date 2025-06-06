#ifndef QT_CODE_FIELD_H
#define QT_CODE_FIELD_H

#include <memory>
#include <set>

#include <QPlainTextEdit>

#include "Id.h"
#include "FilePath.h"
#include "LocationType.h"

class QtHighlighter;
class SourceLocation;
class SourceLocationFile;
class TextCodec;

class QtCodeField: public QPlainTextEdit
{
	Q_OBJECT

public:
	static void clearAnnotationColors();

	QtCodeField(
		size_t startLineNumber,
		const std::string& code,
		std::shared_ptr<SourceLocationFile> locationFile,
		bool convertLocationsOnDemand,
		QWidget* parent = nullptr);

	~QtCodeField() override;

	QSize sizeHint() const override;

	size_t getStartLineNumber() const;
	size_t getEndLineNumber() const;

	int totalLineHeight() const;

	std::string getCode() const;

	std::shared_ptr<SourceLocationFile> getSourceLocationFile() const;
	FilePath getFilePath() const;

	void annotateText();

protected:
	void paintEvent(QPaintEvent* event) override;
	void enterEvent(QEnterEvent* event) override;
	void leaveEvent(QEvent* event) override;

	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

	void keyPressEvent(QKeyEvent* event) override;

	void contextMenuEvent(QContextMenuEvent* event) override;

	virtual void focusTokenIds(const std::vector<Id>& tokenIds);
	virtual void defocusTokenIds(const std::vector<Id>& tokenIds);

	struct Annotation
	{
		int startLine = 0;
		int endLine = 0;

		int startCol = 0;
		int endCol = 0;

		int start = 0;
		int end = 0;

		std::set<Id> tokenIds;
		Id locationId = 0;

		LocationType locationType = LocationType::TOKEN;

		bool isActive = false;
		bool isFocused = false;
		bool isCoFocused = false;
	};

	struct AnnotationColor
	{
		std::string border;
		std::string fill;
		std::string text;
	};

	struct LineColumn
	{
		int line = 0;
		int column = 0;
	};
	
	bool annotateText(
		const std::set<Id>& activeSymbolIds,
		const std::set<Id>& activeLocationIds,
		const std::set<Id>& focusedSymbolIds,
		Id focusedLocationId);

	void createAnnotations(std::shared_ptr<SourceLocationFile> locationFile);
	void activateAnnotations(const std::vector<const Annotation*>& annotations, bool fromMouse, int mouseOffsetX);

	int toTextEditPosition(int lineNumber, int columnNumber) const;
	LineColumn toLineColumn(int textEditPosition) const;

	static int startTextEditPosition();
	int endTextEditPosition() const;

	void setHoveredAnnotations(const std::vector<const Annotation*>& annotations);
	std::vector<QRect> getCursorRectsForAnnotation(const Annotation& annotation) const;

	static const AnnotationColor& getAnnotationColorForAnnotation(const Annotation& annotation);
	void setTextColorForAnnotation(const Annotation& annotation, QColor color) const;

	static const std::string& getFocusColor();

	const Annotation* getAnnotationForLocationId(Id locationId) const;
	std::vector<const Annotation*> getInteractiveAnnotationsForLineNumber(size_t lineNumber) const;
	std::vector<const Annotation*> getInteractiveAnnotationsForPosition(QPoint position) const;
	std::vector<Id> getInteractiveTokenIdsForPosition(QPoint position) const;

	void checkOpenInTabActionEnabled(QPoint position);

	std::vector<Annotation> m_annotations;
	std::vector<const Annotation*> m_hoveredAnnotations;
	std::vector<int> m_linesToRehighlight;

	QAction* m_openInTabAction;

protected slots:
	void openInTab();

private:
	struct ColumnOffset
	{
		int column = 0;
		int offset = 0;
	};
	static std::vector<AnnotationColor> s_annotationColors;
	static std::string s_focusColor;

	void createLineLengthCache();
	void createSurrogateCharacterLocationCache(const QString& code);
	int getColumnCorrectedForSurrogateCharacters(int line, int column) const;

	const size_t m_startLineNumber;
	const std::string m_code;

	std::shared_ptr<SourceLocationFile> m_locationFile;

	std::shared_ptr<QtHighlighter> m_highlighter;

	std::vector<int> m_lineLengths;
	std::vector<std::vector<ColumnOffset>> m_surrogateCharacterLocations;

	int m_endTextEditPosition = 0;

	Id m_openInTabLocationId;
};

#endif	  // QT_CODE_FIELD_H
