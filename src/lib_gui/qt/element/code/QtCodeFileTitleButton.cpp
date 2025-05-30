#include "QtCodeFileTitleButton.h"

#include "Application.h"
#include "ApplicationSettings.h"
#include "ColorScheme.h"
#include "FileSystem.h"
#include "MessageActivateFile.h"
#include "MessageProjectEdit.h"
#include "MessageTabOpenWith.h"
#include "Project.h"
#include "QtContextMenu.h"
#include "QtResources.h"

#include <QMouseEvent>

using namespace std;

QtCodeFileTitleButton::QtCodeFileTitleButton(QWidget* parent)
	: QtSelfRefreshIconButton(QLatin1String(""), FilePath(), "code/file/title", parent)
{
	setObjectName(QStringLiteral("title_button"));
	minimumSizeHint();	  // force font loading

	setFixedHeight(static_cast<int>(std::max(fontMetrics().height() * 1.2, 28.0)));
	setSizePolicy(sizePolicy().horizontalPolicy(), QSizePolicy::Fixed);

	setIconSize(QSize(16, 16));

	connect(this, &QtCodeFileTitleButton::clicked, this, &QtCodeFileTitleButton::clickedTitle);

	m_openInTabAction = new QAction(tr("Open in New Tab"), this);
	m_openInTabAction->setStatusTip(tr("Opens the file in a new tab"));
	m_openInTabAction->setToolTip(tr("Opens the file in a new tab"));
	m_openInTabAction->setEnabled(false);
	connect(m_openInTabAction, &QAction::triggered, this, &QtCodeFileTitleButton::openInTab);

	m_hatchingBackgroundImages["bright"] = QtResources::CODE_VIEW_PATTERN_BRIGHT;
	m_hatchingBackgroundImages["dark"] = QtResources::CODE_VIEW_PATTERN_DARK;
	m_hatchingBackgroundImages["grey"] = QtResources::CODE_VIEW_PATTERN_GREY;
}

const FilePath& QtCodeFileTitleButton::getFilePath() const
{
	return m_filePath;
}

void QtCodeFileTitleButton::setFilePath(const FilePath& filePath)
{
	setEnabled(true);

	m_filePath = filePath;

	updateIcon();
}

void QtCodeFileTitleButton::setModificationTime(const TimeStamp modificationTime)
{
	if (modificationTime.isValid())
	{
		m_modificationTime = modificationTime;
	}
}

void QtCodeFileTitleButton::setProject(const std::string& name)
{
	m_filePath = FilePath();

	setText(QString::fromStdString(name));
	setToolTip(tr("edit project"));

	updateIcon();
}

bool QtCodeFileTitleButton::isComplete() const
{
	return m_isComplete;
}

void QtCodeFileTitleButton::setIsComplete(bool isComplete)
{
	if (m_isComplete == isComplete)
	{
		return;
	}

	m_isComplete = isComplete;
	setProperty("complete", isComplete);

	updateIcon();
}

bool QtCodeFileTitleButton::isIndexed() const
{
	return m_isIndexed;
}

void QtCodeFileTitleButton::setIsIndexed(bool isIndexed)
{
	if (m_isIndexed == isIndexed)
	{
		return;
	}

	m_isIndexed = isIndexed;
	setProperty("nonindexed", !isIndexed);

	updateHatching();
}

void QtCodeFileTitleButton::updateTexts()
{
	if (m_filePath.empty())
	{
		return;
	}

	std::string title = m_filePath.fileName();
	std::string toolTip = "file: " + m_filePath.str();

	if (!m_isIndexed)
	{
		toolTip = "non-indexed " + toolTip;
	}

	if (!m_isComplete)
	{
		toolTip = "incomplete " + toolTip;
	}

	if ((!m_filePath.recheckExists()) ||
		(FileSystem::getLastWriteTime(m_filePath) > m_modificationTime))
	{
		title += "*";
		toolTip = "out-of-date " + toolTip;
	}

	if (ApplicationSettings::getInstance()->getShowDirectoryInCodeFileTitle())
	{
		setAutoElide(true);

		FilePath directoryPath = m_filePath.getParentDirectory();
		std::string directory = directoryPath.str();

		FilePath projectPath = Application::getInstance()->getCurrentProjectPath();
		std::string directoryRelative = directoryPath.getRelativeTo(projectPath).str();

		if (directoryRelative.size() < directory.size())
		{
			directory = directoryRelative;
		}

		if (directory.size() && directory.back() == '/')
		{
			directory.pop_back();
		}

		title = directory + " - " + title;
	}
	else
	{
		setAutoElide(false);
	}

	setText(QString::fromStdString(title));
	setToolTip(QString::fromStdString(toolTip));
}

void QtCodeFileTitleButton::updateFromOther(const QtCodeFileTitleButton* other)
{
	if (!other->m_filePath.empty())
	{
		setFilePath(other->m_filePath);
	}
	else
	{
		setProject(other->text().toStdString());
	}

	setModificationTime(other->m_modificationTime);
	setIsComplete(other->m_isComplete);
	setIsIndexed(other->m_isIndexed);
	updateTexts();
}

void QtCodeFileTitleButton::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::MiddleButton)
	{
		openInTab();
		return;
	}

	QtSelfRefreshIconButton::mouseReleaseEvent(event);
}

void QtCodeFileTitleButton::contextMenuEvent(QContextMenuEvent* event)
{
	FilePath path = m_filePath;
	if (path.empty())
	{
		path = Application::getInstance()->getCurrentProjectPath();
		if (path.empty())
		{
			return;
		}
	}

	m_openInTabAction->setEnabled(!m_filePath.empty());

	QtContextMenu menu(event, this);
	menu.addAction(m_openInTabAction);
	menu.addUndoActions();
	menu.addSeparator();
	menu.addFileActions(path);
	menu.show();
}

void QtCodeFileTitleButton::refresh()
{
	QtSelfRefreshIconButton::refresh();

	updateHatching();
}

void QtCodeFileTitleButton::clickedTitle()
{
	if (!m_filePath.empty())
	{
		MessageActivateFile(m_filePath).dispatch();
	}
	else if (text().size())
	{
		MessageProjectEdit().dispatch();
	}
}

void QtCodeFileTitleButton::openInTab()
{
	if (!m_filePath.empty())
	{
		MessageTabOpenWith(m_filePath).dispatch();
	}
}

void QtCodeFileTitleButton::updateIcon()
{
	if (m_filePath.empty())
	{
		setIconPath(QtResources::CODE_VIEW_EDIT);
	}
	else if (!m_isComplete)
	{
		setIconPath(QtResources::GRAPH_VIEW_FILE_INCOMPLETE);
	}
	else
	{
		setIconPath(QtResources::CODE_VIEW_FILE);
	}
}

void QtCodeFileTitleButton::updateHatching()
{
	if (!m_isIndexed)
	{
		string hatchingColor = ColorScheme::getInstance()->getColor("code/file/title/hatching");
		QString hatchingBackgroundImage = m_hatchingBackgroundImages[hatchingColor];

		setStyleSheet("#title_button { background-image: url(" + hatchingBackgroundImage + "); }");
	}
	else
	{
		setStyleSheet(QLatin1String(""));
	}
}
