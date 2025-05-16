#include "QtStartScreen.h"

#include "ApplicationSettings.h"
#include "MessageLoadProject.h"
#include "ProjectSettings.h"
#include "QtContextMenu.h"
#include "QtMessageBox.h"
#include "QtResources.h"
#include "Version.h"
#include "utilityQt.h"

#include <QCheckBox>
#include <QDesktopServices>
#include <QHBoxLayout>
#include <QLabel>
#include <QString>
#include <QVBoxLayout>

using namespace std;
using namespace utility;

QtRecentProjectButton::QtRecentProjectButton(QWidget *parent)
	: QPushButton(parent)
{
	setMinimumSize(fontMetrics().boundingRect(text()).width() + 45, 40);
	minimumSizeHint(); // force font loading

	setObjectName(QStringLiteral("recentButtonMissing"));
}

void QtRecentProjectButton::setProjectPath(const FilePath &projectFilePath)
{
	m_projectFilePath = projectFilePath;

	setText(QString::fromStdString(m_projectFilePath.withoutExtension().fileName()));
	setFixedWidth(fontMetrics().boundingRect(text()).width() + 45);

	if (m_projectFilePath.exists())
	{
		setToolTip(QString::fromStdString(m_projectFilePath.str()));

		setObjectName(QStringLiteral("recentButton"));
	}
	else
	{
		setToolTip(tr("Couldn't find %1").arg(QString::fromStdString(m_projectFilePath.str())));

		setObjectName(QStringLiteral("recentButtonMissing"));
	}
}

FilePath QtRecentProjectButton::projectPath() const
{
	return m_projectFilePath;
}

void QtRecentProjectButton::contextMenuEvent(QContextMenuEvent *event)
{
	// Allow removing entries from recent projects:

	QIcon removeIcon(QString::fromUtf8(QtResources::STARTSCREEN_DELETE_ICON));
	QAction *removeAction = new QAction(removeIcon, tr("Remove"));
	connect(removeAction, &QAction::triggered, this, &QtRecentProjectButton::removeClicked);

	QtContextMenu menu(event, this);
	menu.addAction(removeAction);
	menu.show();
}


QtStartScreen::QtStartScreen(QWidget *parent)
	: QtWindow(true, parent)
	, m_cppIcon(QString::fromUtf8(QtResources::ICON_CPP_ICON))
	, m_cIcon(QString::fromUtf8(QtResources::ICON_C_ICON))
	, m_pythonIcon(QString::fromUtf8(QtResources::ICON_PYTHON_ICON))
	, m_javaIcon(QString::fromUtf8(QtResources::ICON_JAVA_ICON))
	, m_projectIcon(QString::fromUtf8(QtResources::ICON_EMPTY_ICON))
	, m_githubIcon(QString::fromUtf8(QtResources::STARTSCREEN_GITHUB_ICON))
{
	refreshStyle();
	addLogo();

	QHBoxLayout *layout = new QHBoxLayout();
	layout->setContentsMargins(15, 170, 15, 0);
	layout->setSpacing(1);
	m_content->setLayout(layout);
	{
		QVBoxLayout *col = new QVBoxLayout();
		layout->addLayout(col, 3);

		QLabel *versionLabel = new QLabel(tr("Version %1").arg(QString::fromStdString(Version::getApplicationVersion().toDisplayString())), this);
		versionLabel->setObjectName(QStringLiteral("boldLabel"));

		col->addWidget(versionLabel);
		col->addSpacing(20);

		QPushButton *githubButton = new QPushButton(tr("View on GitHub"), this);
		githubButton->setAttribute(Qt::WA_LayoutUsesWidgetRect); // fixes layouting on Mac
		githubButton->setObjectName(QStringLiteral("infoButton"));
		githubButton->setIcon(m_githubIcon);
		githubButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		connect(githubButton, &QPushButton::clicked, []()
		{
			QDesktopServices::openUrl(QUrl(QStringLiteral("https://github.com/petermost/Sourcetrail"), QUrl::TolerantMode));
		});

		col->addWidget(githubButton);
		col->addSpacing(35);
		col->addStretch();

		QPushButton *newProjectButton = new QPushButton(tr("New Project"), this);
		newProjectButton->setAttribute(Qt::WA_LayoutUsesWidgetRect); // fixes layouting on Mac
		newProjectButton->setObjectName(QStringLiteral("projectButton"));
		newProjectButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		connect(newProjectButton, &QPushButton::clicked, this, &QtStartScreen::openNewProjectDialog);

		col->addWidget(newProjectButton);
		col->addSpacing(8);

		QPushButton *openProjectButton = new QPushButton(tr("Open Project"), this);
		openProjectButton->setAttribute(Qt::WA_LayoutUsesWidgetRect); // fixes layouting on Mac
		openProjectButton->setObjectName(QStringLiteral("projectButton"));
		openProjectButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		connect(openProjectButton, &QPushButton::clicked, this, &QtStartScreen::openOpenProjectDialog);

		col->addWidget(openProjectButton);
	}

	layout->addSpacing(50);

	{
		QVBoxLayout *col = new QVBoxLayout();
		layout->addLayout(col, 1);

		QLabel *recentProjectsLabel = new QLabel(tr("Recent Projects: "), this);
		recentProjectsLabel->setObjectName(QStringLiteral("titleLabel"));

		col->addWidget(recentProjectsLabel);
		col->addSpacing(20);

		// Create all 'recent' buttons and show/fill them later:
		for (size_t i = 0; i < ApplicationSettings::getInstance()->getMaxRecentProjectsCount(); ++i)
		{
			QtRecentProjectButton *button = new QtRecentProjectButton(this);
			button->setAttribute(Qt::WA_LayoutUsesWidgetRect); // fixes layouting on Mac
			button->setIcon(m_projectIcon);
			button->setIconSize(QSize(30, 30));
			connect(button, &QtRecentProjectButton::clicked, this, &QtStartScreen::handleButtonClicked);
			connect(button, &QtRecentProjectButton::removeClicked, this, &QtStartScreen::handleButtonRemoved);

			col->addWidget(button);

			m_recentProjectsButtons.push_back(button);
		}
		col->addStretch();
	}
	layout->addStretch(1);

	updateButtons();

	QSize size = sizeHint();
	move(parentWidget()->width() / 2 - size.width() / 2, parentWidget()->height() / 2 - size.height() / 2);
}

void QtStartScreen::handleButtonClicked()
{
	QtRecentProjectButton *button = qobject_cast<QtRecentProjectButton *>(sender());
	FilePath recentProjectPath = button->projectPath();

	if (recentProjectPath.exists())
	{
		MessageLoadProject(recentProjectPath).dispatch();

		emit finished();
	}
	else
	{
		QtMessageBox msgBox(this);
		msgBox.setWindowTitle(tr("Missing Project File"));
		msgBox.setText(tr("<strong>Missing Project File</strong>"));
		msgBox.setInformativeText(tr("<p>Couldn't find \"%1\"</p><p>Do you want to remove it from recent projects?</p>").arg(button->text()));
		msgBox.setDetailedText(tr("Couldn't find %1").arg(QString::fromStdString(recentProjectPath.str())));

		QPushButton *removeButton = msgBox.addButton(tr("Remove"), QtMessageBox::YesRole);
		msgBox.addButton(tr("Keep"), QtMessageBox::NoRole);
		msgBox.setIcon(QtMessageBox::Question);

		if (msgBox.execModal() == removeButton)
			removeRecentProjectPath(recentProjectPath);
	}
}

void QtStartScreen::handleButtonRemoved()
{
	removeRecentProjectPath(qobject_cast<QtRecentProjectButton *>(sender())->projectPath());
}

QSize QtStartScreen::sizeHint() const
{
	return QSize(600, 650);
}

void QtStartScreen::removeRecentProjectPath(const FilePath &projectFilePath)
{
	vector<FilePath> recentProjects = ApplicationSettings::getInstance()->getRecentProjects();
	for (auto recentProject = recentProjects.begin(); recentProject != recentProjects.end(); ++recentProject)
	{
		if (recentProject->str() == projectFilePath.str())
		{
			recentProjects.erase(recentProject);
			ApplicationSettings::getInstance()->setRecentProjects(recentProjects);
			ApplicationSettings::getInstance()->save();
			break;
		}
	}
	updateButtons();
}

void QtStartScreen::updateButtons()
{
	vector<FilePath> recentProjects = ApplicationSettings::getInstance()->getRecentProjects();
	for (size_t i = 0; i < m_recentProjectsButtons.size(); ++i)
	{
		QtRecentProjectButton *button = m_recentProjectsButtons[i];
		if (i < recentProjects.size())
		{
			FilePath recentProjectPath = recentProjects[i];

			button->setProjectPath(recentProjectPath);
			setButtonIcon(button, ProjectSettings::getLanguageOfProject(recentProjectPath));
		}
		else
			button->hide();
	}
	refreshStyle();
}

void QtStartScreen::refreshStyle()
{
	setStyleSheet(QtResources::loadStyleSheet(QtResources::STARTSCREEN_CSS));
}

void QtStartScreen::setButtonIcon(QtRecentProjectButton *button, LanguageType languageType) const
{
	switch (languageType)
	{
#if BUILD_CXX_LANGUAGE_PACKAGE
	case LanguageType::C:
		button->setIcon(m_cIcon);
		break;
	case LanguageType::CXX:
		button->setIcon(m_cppIcon);
		break;
#endif // BUILD_CXX_LANGUAGE_PACKAGE
#if BUILD_JAVA_LANGUAGE_PACKAGE
	case LanguageType::JAVA:
		button->setIcon(m_javaIcon);
		break;
#endif // BUILD_JAVA_LANGUAGE_PACKAGE
#if BUILD_PYTHON_LANGUAGE_PACKAGE
	case LanguageType::PYTHON:
		button->setIcon(m_pythonIcon);
		break;
#endif // BUILD_PYTHON_LANGUAGE_PACKAGE
	case LanguageType::CUSTOM:
	default:
		button->setIcon(m_projectIcon);
		break;
	}
}
