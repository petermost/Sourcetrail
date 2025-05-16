#ifndef QT_BOOKMARK_CREATOR_H
#define QT_BOOKMARK_CREATOR_H

#include <QWidget>

#include "BookmarkController.h"
#include "ControllerProxy.h"
#include "QtBookmarkWindow.h"

class BookmarkCategory;
class QComboBox;
class QLineEdit;
class QTextEdit;

class QtBookmarkCreator: public QtBookmarkWindow
{
	Q_OBJECT

public:
	QtBookmarkCreator(ControllerProxy<BookmarkController>* controllerProxy, QWidget* parent = nullptr, BookmarkId bookmarkId = BookmarkId::NONE);
	
	void setDisplayName(const std::string& name);
	void setComment(const std::string& comment);
	
	void setBookmarkCategories(const std::vector<BookmarkCategory>& categories);
	void setCurrentBookmarkCategory(const BookmarkCategory& category);
	
	void setNodeId(Id nodeId);
	
protected:
	void handleNext() override;
	void handleClose() override;
	
private slots:
	void onNameChanged(const QString& text);
	
private:
	const BookmarkId m_editBookmarkId;

	QLineEdit* m_displayName;
	QTextEdit* m_commentBox;
	QComboBox* m_categoryBox;

	Id m_nodeId;
};

#endif	  // QT_BOOKMARK_CREATOR_H
