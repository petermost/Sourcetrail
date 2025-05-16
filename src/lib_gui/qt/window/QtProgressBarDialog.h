#ifndef QT_PROGRESS_BAR_DIALOG_H
#define QT_PROGRESS_BAR_DIALOG_H

#include <functional>

#include "QtIndexingDialog.h"
#include "RefreshInfo.h"

class QCheckBox;
class QLabel;
class QRadioButton;
class QtProgressBar;

class QtProgressBarDialog: public QtIndexingDialog
{
	Q_OBJECT

public:
	QtProgressBarDialog(float topRatio, bool hideable, QWidget* parent = nullptr);
	void updateTitle(const QString& title);
	std::string getTitle() const;
	void updateMessage(const QString& message);
	std::string getMessage() const;
	void setUnknownProgress();
	void updateProgress(size_t progress);
	size_t getProgress() const;

protected:
	void resizeEvent(QResizeEvent* event) override;
	virtual void setGeometries();

private:
	QLabel* m_title = nullptr;
	QWidget* m_top;
	float m_topRatio;
	QtProgressBar* m_progressBar;
	QLabel* m_percentLabel;
	QLabel* m_messageLabel;
};

#endif	  // QT_PROGRESS_BAR_DIALOG_H
