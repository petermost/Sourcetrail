#ifndef QT_BOOKMARK_H
#define QT_BOOKMARK_H

#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QTreeWidget>

#include "Bookmark.h"
#include "BookmarkController.h"
#include "ControllerProxy.h"

class Bookmark;

class QtBookmark: public QFrame
{
	Q_OBJECT

public:
	QtBookmark(ControllerProxy<BookmarkController>* controllerProxy);
	~QtBookmark() override;

	void setBookmark(const std::shared_ptr<Bookmark> bookmark);
	Id getBookmarkId() const;

	QTreeWidgetItem* getTreeWidgetItem() const;
	void setTreeWidgetItem(QTreeWidgetItem* treeWidgetItem);

public slots:
	void commentToggled();

protected:
	void resizeEvent(QResizeEvent* event) override;
	void showEvent(QShowEvent* event) override;

<<<<<<< HEAD
	void enterEvent(QEnterEvent* event) override;
	void leaveEvent(QEvent* event) override;
=======
	virtual void enterEvent(QEnterEvent* event) override;
	virtual void leaveEvent(QEvent* event) override;
>>>>>>> 09ccbe42a1120f7185e91e13d9d2b8583217be7f

private slots:
	void activateClicked();
	void editClicked();
	void deleteClicked();
	void elideButtonText();

private:
	void updateArrow();

	std::string getDateString() const;

	ControllerProxy<BookmarkController>* m_controllerProxy;

	QPushButton* m_activateButton;
	QPushButton* m_editButton;
	QPushButton* m_deleteButton;
	QPushButton* m_toggleCommentButton;

	QLabel* m_comment;
	QLabel* m_dateLabel;

	std::shared_ptr<Bookmark> m_bookmark;

	// pointer to the bookmark category item in the treeView, allows to refresh tree view when a
	// node changes in size (e.g. toggle comment). Not a nice solution to the problem, but couldn't
	// find anything better yet. (sizeHintChanged signal can't be emitted here...)
	QTreeWidgetItem* m_treeWidgetItem = nullptr;

	std::wstring m_arrowImageName;
	bool m_hovered = false;

	bool m_ignoreNextResize = false;
};

#endif	  // QT_BOOKMARK_H
