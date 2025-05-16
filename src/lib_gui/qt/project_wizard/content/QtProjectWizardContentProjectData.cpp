#include "QtProjectWizardContentProjectData.h"
#include "QtMessageBox.h"

#include <QRegularExpression>
#include <boost/filesystem/path.hpp>

#include "FileSystem.h"
#include "ProjectSettings.h"

QtProjectWizardContentProjectData::QtProjectWizardContentProjectData(
	std::shared_ptr<ProjectSettings> projectSettings,
	QtProjectWizardWindow* window,
	bool disableNameEditing)
	: QtProjectWizardContent(window)
	, m_projectSettings(projectSettings)
	, m_disableNameEditing(disableNameEditing)
{
	setIsRequired(true);
}

void QtProjectWizardContentProjectData::populate(QGridLayout* layout, int& row)
{
	QLabel* nameLabel = createFormLabel(QStringLiteral("Sourcetrail Project Name"));
	m_projectName = new QLineEdit();
	m_projectName->setObjectName(QStringLiteral("name"));
	m_projectName->setAttribute(Qt::WA_MacShowFocusRect, false);
	m_projectName->setEnabled(!m_disableNameEditing);
	connect(
		m_projectName,
		&QLineEdit::textEdited,
		this,
		&QtProjectWizardContentProjectData::onProjectNameEdited);

	layout->addWidget(nameLabel, row, QtProjectWizardWindow::FRONT_COL, Qt::AlignRight);
	layout->addWidget(m_projectName, row, QtProjectWizardWindow::BACK_COL);
	layout->setRowMinimumHeight(row, 30);
	row++;

	QLabel* locationLabel = createFormLabel(QStringLiteral("Sourcetrail Project Location"));
	m_projectFileLocation = new QtLocationPicker(this);
	m_projectFileLocation->setPickDirectory(true);
	m_projectFileLocation->setEnabled(!m_disableNameEditing);

	layout->addWidget(locationLabel, row, QtProjectWizardWindow::FRONT_COL, Qt::AlignRight);
	layout->addWidget(m_projectFileLocation, row, QtProjectWizardWindow::BACK_COL, Qt::AlignTop);
	addHelpButton(
		QStringLiteral("Sourcetrail Project Location"),
		QStringLiteral("The directory the Sourcetrail project file (.srctrlprj) will be saved to."),
		layout,
		row);
	layout->setRowMinimumHeight(row, 30);
	row++;
}

void QtProjectWizardContentProjectData::load()
{
	m_projectName->setText(QString::fromStdString(m_projectSettings->getProjectName()));
	m_projectFileLocation->setText(
		QString::fromStdString(m_projectSettings->getProjectDirectoryPath().str()));
}

void QtProjectWizardContentProjectData::save()
{
	m_projectSettings->setProjectFilePath(
		m_projectName->text().toStdString(),
		FilePath(m_projectFileLocation->getText().toStdString()));
}

bool QtProjectWizardContentProjectData::check()
{
	if (m_projectName->text().isEmpty())
	{
		QtMessageBox msgBox(m_window);
		msgBox.setText(QStringLiteral("Please enter a project name."));
		msgBox.execModal();
		return false;
	}

	if (!boost::filesystem::portable_file_name(m_projectName->text().toStdString()))
	{
		QtMessageBox msgBox(m_window);
		msgBox.setText(
			"The provided project name is not a valid file name. Please adjust the name "
			"accordingly.");
		msgBox.execModal();
		return false;
	}

	if (m_projectFileLocation->getText().isEmpty())
	{
		QtMessageBox msgBox(m_window);
		msgBox.setText(
			QStringLiteral("Please define the location for the Sourcetrail project file."));
		msgBox.execModal();
		return false;
	}

	std::vector<FilePath> paths =
		FilePath(m_projectFileLocation->getText().toStdString()).expandEnvironmentVariables();
	if (paths.size() != 1)
	{
		QtMessageBox msgBox(m_window);
		msgBox.setText(
			"The specified location seems to be invalid. Please make sure that the used "
			"environment variables are unambiguous.");
		msgBox.execModal();
		return false;
	}
	else if (!paths.front().isAbsolute())
	{
		QtMessageBox msgBox(m_window);
		msgBox.setText(
			"The specified location seems to be invalid. Please specify an absolute directory "
			"path.");
		msgBox.execModal();
		return false;
	}
	else if (!paths.front().isValid())
	{
		QtMessageBox msgBox(m_window);
		msgBox.setText(
			"The specified location seems to be invalid. Please check the characters used in the "
			"path.");
		msgBox.execModal();
		return false;
	}
	else if (!paths[0].exists())
	{
		QtMessageBox msgBox(m_window);
		msgBox.setText(QStringLiteral(
			"The specified location does not exist. Do you want to create the directory?"));
		msgBox.addButton(QStringLiteral("Abort"), QtMessageBox::ButtonRole::NoRole);
		QPushButton* createButton = msgBox.addButton(
			QStringLiteral("Create"), QtMessageBox::ButtonRole::YesRole);
		msgBox.setDefaultButton(createButton);
		msgBox.setIcon(QtMessageBox::Icon::Question);
		if (msgBox.execModal() == createButton)	 // QtMessageBox::Yes
		{
			FileSystem::createDirectories(paths[0]);
		}
		else
		{
			return false;
		}
	}

	return true;
}

void QtProjectWizardContentProjectData::onProjectNameEdited(QString text)
{
	const QRegularExpression regex(QStringLiteral("[^A-Za-z0-9_.-]"));

	const int cursorPosition = m_projectName->cursorPosition();

	text.replace(regex, QStringLiteral("_"));

	m_projectName->setText(text);
	m_projectName->setCursorPosition(cursorPosition);
}
