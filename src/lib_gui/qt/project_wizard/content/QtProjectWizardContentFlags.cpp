#include "QtProjectWizardContentFlags.h"
#include "QtMessageBox.h"

#include <QFormLayout>

#include "QtStringListBox.h"
#include "SourceGroupSettingsWithCxxPathsAndFlags.h"
#include "ToolChain.h"
#include "utilityString.h"

using namespace std;

QtProjectWizardContentFlags::QtProjectWizardContentFlags(
	std::shared_ptr<SourceGroupSettingsWithCxxPathsAndFlags> settings,
	QtProjectWizardWindow* window,
	bool indicateAsAdditional)
	: QtProjectWizardContent(window)
	, m_settings(settings)
	, m_indicateAsAdditional(indicateAsAdditional)
{
}

void QtProjectWizardContentFlags::populate(QGridLayout* layout, int& row)
{
	const QString labelText(
		(std::string(m_indicateAsAdditional ? "Additional " : "") + "Compiler Flags").c_str());
	QLabel* label = createFormLabel(labelText);
	layout->addWidget(label, row, QtProjectWizardWindow::FRONT_COL, Qt::AlignTop);

	addHelpButton(
		labelText,
		QStringLiteral(
			"<p>Define additional Clang compiler flags used during indexing. Here are some "
			"examples:</p>"
			"<ul style=\"-qt-list-indent:0;\">"
			"<li style=\"margin-left:1em\">use '-DRELEASE' to add a preprocessor #define for "
			"'RELEASE'</li>"
			"<li style=\"margin-left:1em\">use '-U__clang__' to remove the preprocessor #define "
			"for "
			"'__clang__'</li>"
			"<li style=\"margin-left:1em\">use '-DFOO=900' to add an integer preprocessor "
			"define</li>"
			"<li style=\"margin-left:1em\">use '-DFOO=\"bar\"' to add a string preprocessor "
			"define</li>"
			"</ul>"),
		layout,
		row);

	m_list = new QtStringListBox(this, label->text());
	layout->addWidget(m_list, row, QtProjectWizardWindow::BACK_COL);
	row++;
}

void QtProjectWizardContentFlags::load()
{
	m_list->setStrings(m_settings->getCompilerFlags());
}

void QtProjectWizardContentFlags::save()
{
	m_settings->setCompilerFlags(m_list->getStrings());
}

bool QtProjectWizardContentFlags::check()
{
	std::string error;

	for (const std::string& flag: m_list->getStrings())
	{
		if (utility::isPrefix(ClangCompiler::forceIncludeOption() + " "s, flag) ||
			utility::isPrefix(ClangCompiler::forceIncludeOption2() + " "s, flag))
		{
			error = "The entered compiler flag \"" + flag + "\" contains an error. Please remove the intermediate space character.\n";
		}
	}

	if (!error.empty())
	{
		QtMessageBox msgBox(m_window);
		msgBox.setText(QString::fromStdString(error));
		msgBox.execModal();
		return false;
	}

	return true;
}
