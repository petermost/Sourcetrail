#ifndef QT_WINDOW_H
#define QT_WINDOW_H

#include <QHBoxLayout>
#include <QResizeEvent>
#include <QSizeGrip>
#include <QWidget>

#include "QtWindowBase.h"
#include "QtWindowStack.h"

class QLabel;
class QPushButton;
class QVBoxLayout;

class QtWindow: public QtWindowBase
{
	Q_OBJECT

public:
	QtWindow(bool isSubWindow, QWidget* parent = nullptr);

	void setup();

	void setCancelAble(bool cancelAble);
	void setScrollAble(bool scrollAble);

	bool isScrollAble() const;

	void updateTitle(const QString& title);
	std::string getTitle() const;
	void updateSubTitle(const QString& subTitle);

	void updateNextButton(const QString& text);
	void updatePreviousButton(const QString& text);
	void updateCloseButton(const QString& text);

	void setNextEnabled(bool enabled);
	void setPreviousEnabled(bool enabled);
	void setCloseEnabled(bool enabled);

	void setNextVisible(bool visible);
	void setPreviousVisible(bool visible);
	void setCloseVisible(bool visible);

	void setNextDefault(bool isDefault);
	void setPreviousDefault(bool isDefault);
	void setCloseDefault(bool isDefault);

signals:
	void finished();
	void canceled();

	void next();
	void previous();

protected:
	void closeEvent(QCloseEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;

	virtual void populateWindow(QWidget* widget);
	virtual void windowReady();

	virtual void handleNext();
	virtual void handlePrevious();
	virtual void handleClose();

	void setupDone();
	void addLogo();
	QHBoxLayout* createButtons();

	QLabel* m_title = nullptr;
	QLabel* m_subTitle = nullptr;

	QPushButton* m_nextButton = nullptr;
	QPushButton* m_previousButton = nullptr;
	QPushButton* m_closeButton = nullptr;

public slots:
	void handleNextPress(bool);
	void handlePreviousPress();
	void handleClosePress();

private:
	bool m_cancelAble = true;
	bool m_scrollAble = false;

	bool m_hasLogo = false;
};

#endif	  // QT_WINDOW_H
