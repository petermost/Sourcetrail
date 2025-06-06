#include "QtProjectWizardContentJavaStandard.h"

#include <QComboBox>
#include <QLabel>

#include "SourceGroupSettingsWithJavaStandard.h"
#include "ToolVersionSupport.h"

QtProjectWizardContentJavaStandard::QtProjectWizardContentJavaStandard(
	std::shared_ptr<SourceGroupSettingsWithJavaStandard> sourceGroupSettings,
	QtProjectWizardWindow* window)
	: QtProjectWizardContent(window), m_sourceGroupSettings(sourceGroupSettings)
{
}

void QtProjectWizardContentJavaStandard::populate(QGridLayout* layout, int& row)
{
	m_standard = new QComboBox();
	layout->addWidget(
		createFormLabel("Java Standard"), row, QtProjectWizardWindow::FRONT_COL, Qt::AlignRight);
	layout->addWidget(m_standard, row, QtProjectWizardWindow::BACK_COL, Qt::AlignLeft);
	row++;
}

void QtProjectWizardContentJavaStandard::load()
{
	m_standard->clear();

	if (m_sourceGroupSettings)
	{
		std::vector<std::string> standards = EclipseVersionSupport::getAvailableJavaStandards();
		for (size_t i = 0; i < standards.size(); i++)
		{
			m_standard->insertItem(static_cast<int>(i), QString::fromStdString(standards[i]));
		}

		m_standard->setCurrentText(QString::fromStdString(m_sourceGroupSettings->getJavaStandard()));
	}
}

void QtProjectWizardContentJavaStandard::save()
{
	if (m_sourceGroupSettings)
	{
		m_sourceGroupSettings->setJavaStandard(m_standard->currentText().toStdString());
	}
}
