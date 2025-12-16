#ifndef QT_START_SCREEN_H
#define QT_START_SCREEN_H

#include <QPushButton>

#include "FilePath.h"
#include "LanguageType.h"
#include "QtWindow.h"

class QtRecentProjectButton: public QPushButton
{
	Q_OBJECT

public:
	explicit QtRecentProjectButton(QWidget* parent);
	
	void setProjectPath(const FilePath& projectFilePath);
	FilePath projectPath() const;

signals:
	void removeClicked();
	
protected:
	void contextMenuEvent(QContextMenuEvent* event) override;
	
private:
	FilePath m_projectFilePath;
};


class QtStartScreen: public QtWindow
{
	Q_OBJECT

public:
	explicit QtStartScreen(QWidget *parent);
	
	QSize sizeHint() const override;

signals:
	void openOpenProjectDialog();
	void openNewProjectDialog();

private slots:
	void handleButtonClicked();
	void handleButtonRemoved();
	
private:
	void removeRecentProjectPath(const FilePath &projectFilePath);
	void updateButtons();
	void refreshStyle();
	
	void setButtonIcon(QtRecentProjectButton *button, LanguageType languageType) const;
	
	std::vector<QtRecentProjectButton*> m_recentProjectsButtons;
	
	const QIcon m_cppIcon;
	const QIcon m_cIcon;
	const QIcon m_javaIcon;
	const QIcon m_projectIcon;
	const QIcon m_githubIcon;
};

#endif	  // QT_START_SCREEN_H
