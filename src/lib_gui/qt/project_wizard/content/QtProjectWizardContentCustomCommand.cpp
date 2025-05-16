#include "QtProjectWizardContentCustomCommand.h"
#include "QtMessageBox.h"

#include <QCheckBox>
#include <QLineEdit>
#include <boost/filesystem/path.hpp>

#include "FileSystem.h"
#include "ProjectSettings.h"
#include "SourceGroupSettingsCustomCommand.h"
#include "SqliteIndexStorage.h"

QtProjectWizardContentCustomCommand::QtProjectWizardContentCustomCommand(
	std::shared_ptr<SourceGroupSettingsCustomCommand> settings, QtProjectWizardWindow* window)
	: QtProjectWizardContent(window), m_settings(settings) 
{
}

void QtProjectWizardContentCustomCommand::populate(QGridLayout* layout, int& row)
{
	QLabel* nameLabel = createFormLabel(QStringLiteral("Custom Command"));
	addHelpButton(
		QStringLiteral("Custom Command"),
		"<p>Specify the commandline call that will be executed for each source file in this Source "
		"Group. "
		"You can use the following variables, %{SOURCE_FILE_PATH} is mandatory.</p>"
		"<ul>"
		"<li><b>%{SOURCE_FILE_PATH}</b> - Path to each source file (mandatory)</li>"
		"<li><b>%{DATABASE_FILE_PATH}</b> - Path to database file: \"" +
			QString::fromStdString(m_settings->getProjectSettings()->getTempDBFilePath().str()) +
			"\"</li>"
			"<li><b>%{DATABASE_VERSION}</b> - Database version used by this Sourcetrail version: "
			"\"" +
			QString::number(SqliteIndexStorage::getStorageVersion()) +
			"\"</li>"
			"<li><b>%{PROJECT_FILE_PATH}</b> - Path to project file: \"" +
			QString::fromStdString(m_settings->getProjectSettings()->getProjectFilePath().str()) +
			"\"</li>"
			"</ul>",
		layout,
		row);

	m_customCommand = new QLineEdit();
	m_customCommand->setObjectName(QStringLiteral("name"));
	m_customCommand->setAttribute(Qt::WA_MacShowFocusRect, false);
	m_runInParallel = new QCheckBox(QStringLiteral("Run in Parallel"));

	layout->setRowMinimumHeight(row, 30);

	layout->addWidget(nameLabel, row, QtProjectWizardWindow::FRONT_COL, Qt::AlignRight);
	layout->addWidget(m_customCommand, row, QtProjectWizardWindow::BACK_COL);
	row++;

	layout->addWidget(m_runInParallel, row, QtProjectWizardWindow::BACK_COL);
	row++;
}

void QtProjectWizardContentCustomCommand::load()
{
	m_customCommand->setText(QString::fromStdString(m_settings->getCustomCommand()));
	m_runInParallel->setChecked(m_settings->getRunInParallel());
}

void QtProjectWizardContentCustomCommand::save()
{
	m_settings->setCustomCommand(m_customCommand->text().toStdString());
	m_settings->setRunInParallel(m_runInParallel->isChecked());
}

bool QtProjectWizardContentCustomCommand::check()
{
	if (m_customCommand->text().isEmpty())
	{
		QtMessageBox msgBox(m_window);
		msgBox.setText(QStringLiteral("Please enter a custom command."));
		msgBox.execModal();
		return false;
	}

	if (m_customCommand->text().toStdString().find("%{SOURCE_FILE_PATH}") == std::string::npos)
	{
		QtMessageBox msgBox(m_window);
		msgBox.setText(QStringLiteral("The variable %{SOURCE_FILE_PATH} is missing in the custom command."));
		msgBox.execModal();
		return false;
	}

	return true;
}
