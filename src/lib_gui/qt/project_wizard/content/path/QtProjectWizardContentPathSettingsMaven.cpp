#include "QtProjectWizardContentPathSettingsMaven.h"

#include <QCheckBox>

#include "SourceGroupSettingsJavaMaven.h"

QtProjectWizardContentPathSettingsMaven::QtProjectWizardContentPathSettingsMaven(
	std::shared_ptr<SourceGroupSettingsJavaMaven> settings, QtProjectWizardWindow* window)
	: QtProjectWizardContentPath(window), m_settings(settings)
{
	setTitleString("Maven Settings File (settings.xml)");
	setHelpString(
		"If your project uses a custom Maven settings file, specify it here. "
		"If you leave this option empty, the default Maven settings will be used.<br />"
		"<br />"
		"You can make use of environment variables with ${ENV_VAR}.");
	setPlaceholderString("Use Default");
	setFileEndings({".xml"});
}

void QtProjectWizardContentPathSettingsMaven::populate(QGridLayout* layout, int& row)
{
	QtProjectWizardContentPath::populate(layout, row);
	m_picker->setPickDirectory(false);
	m_picker->setFileFilter("Settings File (*.xml)");
}

void QtProjectWizardContentPathSettingsMaven::load()
{
	m_picker->setText(QString::fromStdString(m_settings->getMavenSettingsFilePath().str()));
}

void QtProjectWizardContentPathSettingsMaven::save()
{
	m_settings->setMavenSettingsFilePath(FilePath(m_picker->getText().toStdString()));
}

std::shared_ptr<SourceGroupSettings> QtProjectWizardContentPathSettingsMaven::getSourceGroupSettings()
{
	return m_settings;
}
