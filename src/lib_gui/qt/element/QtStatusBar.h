#ifndef QT_STATUS_BAR_H
#define QT_STATUS_BAR_H

#include <memory>
#include <string>

#include <QLabel>
#include <QPushButton>
#include <QStatusBar>

#include "ErrorCountInfo.h"

class QProgressBar;

class QtStatusBar: public QStatusBar
{
	Q_OBJECT

public:
	QtStatusBar();

	void setText(const std::string& text, bool isError, bool showLoader);
	void setErrorCount(ErrorCountInfo errorCount);

	void setIdeStatus(const std::string& text);

	void showIndexingProgress(size_t progressPercent);
	void hideIndexingProgress();

protected:
	void resizeEvent(QResizeEvent* event) override;

private slots:
	static void showStatus();
	static void showErrors();
	static void clickedIndexingProgress();

private:
	QWidget* addPermanentVLine();

	std::shared_ptr<QMovie> m_movie;

	std::string m_textString;

	QPushButton m_text;
	QLabel m_loader;
	QPushButton m_errorButton;

	QLabel m_ideStatusText;

	QPushButton* m_indexingStatus;
	QProgressBar* m_indexingProgress;

	QWidget* m_vlineError;
	QWidget* m_vlineIndexing;
};

#endif	  // QT_STATUS_BAR_H
