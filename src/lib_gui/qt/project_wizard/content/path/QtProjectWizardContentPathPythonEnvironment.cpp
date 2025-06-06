#include "QtProjectWizardContentPathPythonEnvironment.h"

#include "ResourcePaths.h"
#include "SourceGroupSettingsPythonEmpty.h"
#include "utilityApp.h"
#include "utilityFile.h"

using namespace boost::chrono;

QtProjectWizardContentPathPythonEnvironment::QtProjectWizardContentPathPythonEnvironment(
	std::shared_ptr<SourceGroupSettingsPythonEmpty> settings, QtProjectWizardWindow* window)
	: QtProjectWizardContentPath(window), m_settings(settings)
{
	setTitleString("Python Environment");
	setHelpString(
		"Here you can specify the path to the directory or to the executable of the (virtual) "
		"Python environment that should be used to resolve "
		"dependencies within the indexed source code. <br />"
		"<br />"
		"If you would run:<br />"
		"<br />"
		"$ cd C:\\dev\\python\\envs<br />"
		"$ virtualenv py37<br />"
		"<br />"
		"you would set it to \"C:\\dev\\python\\envs\\py37\" or "
		"\"C:\\dev\\python\\envs\\py37\\Scripts\\python.exe\". <br />"
		"Leave blank to use the default Python environment. You can make use of environment "
		"variables with ${ENV_VAR}.");
	setPlaceholderString("Use Default");
}

void QtProjectWizardContentPathPythonEnvironment::populate(QGridLayout* layout, int& row)
{
	QtProjectWizardContentPath::populate(layout, row);
	connect(
		m_picker,
		&QtLocationPicker::textChanged,
		this,
		&QtProjectWizardContentPathPythonEnvironment::onTextChanged);

	m_resultLabel = new QLabel();
	m_resultLabel->setWordWrap(true);
	layout->addWidget(m_resultLabel, row, QtProjectWizardWindow::BACK_COL);
	row++;
}

void QtProjectWizardContentPathPythonEnvironment::load()
{
	m_picker->setText(QString::fromStdString(m_settings->getEnvironmentPath().str()));
}

void QtProjectWizardContentPathPythonEnvironment::save()
{
	m_settings->setEnvironmentPath(FilePath(m_picker->getText().toStdString()));
}

void QtProjectWizardContentPathPythonEnvironment::onTextChanged(const QString& text)
{
	if (text.isEmpty())
	{
		m_resultLabel->clear();
	}
	else
	{
		m_resultLabel->setText("Checking validity of Python environment...");
		std::thread([=, this]() {
			const utility::ProcessOutput out = utility::executeProcess(
				ResourcePaths::getPythonIndexerFilePath().str(),
				{"check-environment",
				 "--environment-path",
				 utility::getExpandedAndAbsolutePath(
					 FilePath(text.toStdString()), m_settings->getProjectDirectoryPath())
					 .str()},
				FilePath(),
				false,
				milliseconds(5000));
			m_onQtThread([=, this]() {
				if (out.exitCode == 0)
				{
					m_resultLabel->setText(QString::fromStdString(out.output));
				}
				else
				{
					m_resultLabel->setText(
						"An error occurred while checking environment path. Unable to check "
						"validity.");
				}
			});
		}).detach();
	}
}

std::shared_ptr<SourceGroupSettings> QtProjectWizardContentPathPythonEnvironment::getSourceGroupSettings()
{
	return m_settings;
}
