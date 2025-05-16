#ifndef QT_CODE_FILE_TITLE_BUTTON_H
#define QT_CODE_FILE_TITLE_BUTTON_H

#include "FilePath.h"
#include "QtSelfRefreshIconButton.h"
#include "TimeStamp.h"

#include <string>
#include <map>

class QtCodeFileTitleButton: public QtSelfRefreshIconButton
{
	Q_OBJECT

public:
	QtCodeFileTitleButton(QWidget* parent = nullptr);
	~QtCodeFileTitleButton() override = default;

	const FilePath& getFilePath() const;
	void setFilePath(const FilePath& filePath);

	void setModificationTime(const TimeStamp modificationTime);
	void setProject(const std::string& name);

	bool isComplete() const;
	void setIsComplete(bool isComplete);

	bool isIndexed() const;
	void setIsIndexed(bool isIndexed);

	void updateTexts();
	void updateFromOther(const QtCodeFileTitleButton* other);

protected:
	void mouseReleaseEvent(QMouseEvent* event) override;
	void contextMenuEvent(QContextMenuEvent* event) override;

	void refresh() override;

private slots:
	void clickedTitle();
	void openInTab();

private:
	void updateIcon();
	void updateHatching();

	std::map<std::string, QString> m_hatchingBackgroundImages;

	FilePath m_filePath;
	TimeStamp m_modificationTime;
	bool m_isComplete = true;
	bool m_isIndexed = true;

	QAction* m_openInTabAction;
};

#endif	  // QT_CODE_FILE_TITLE_BUTTON_H
