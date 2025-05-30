#include "QtProjectWizardContentSelect.h"

#include "LanguageType.h"
#include "QtFlowLayout.h"
#include "QtMessageBox.h"
#include "QtProjectWizardWindow.h"
#include "QtResources.h"
#include "ResourcePaths.h"
#include "SqliteIndexStorage.h"
#include "utilityApp.h"
#include "utilityQt.h"
#include "utilityString.h"

#include <QButtonGroup>
#include <QLabel>
#include <QPushButton>

#include <regex>

using namespace utility;
using namespace boost::chrono;

QtProjectWizardContentSelect::QtProjectWizardContentSelect(QtProjectWizardWindow* window)
	: QtProjectWizardContent(window)
{
}

void QtProjectWizardContentSelect::populate(QGridLayout* layout, int&  /*row*/)
{
	std::string pythonIndexerVersion = " ";
	{
		utility::ProcessOutput output = utility::executeProcess(
			ResourcePaths::getPythonIndexerFilePath().str(), {"--version"}, FilePath(), false, milliseconds(5000));
		if (output.exitCode == 0)
		{
			std::string str = output.output;
			std::regex regex("v\\d*\\.db\\d*\\.p\\d*");	   // "\\d" matches any digit; "\\." matches
														   // the "." character
			std::smatch matches;
			std::regex_search(str, matches, regex);
			if (!matches.empty())
			{
				pythonIndexerVersion = matches.str(0) + " ";
			}
		}
	}

	struct SourceGroupInfo
	{
		SourceGroupInfo(SourceGroupType type, bool recommended = false)
			: type(type), recommended(recommended)
		{
		}
		const SourceGroupType type;
		const bool recommended;
	};

	// define which kind of source groups are available for each language
	std::map<LanguageType, std::vector<SourceGroupInfo>> sourceGroupInfos;
#if BUILD_CXX_LANGUAGE_PACKAGE
	sourceGroupInfos[LanguageType::C].push_back(SourceGroupInfo(SourceGroupType::CXX_CDB, true));
	sourceGroupInfos[LanguageType::C].push_back(SourceGroupInfo(SourceGroupType::CXX_VS));
	sourceGroupInfos[LanguageType::C].push_back(SourceGroupInfo(SourceGroupType::CXX_CODEBLOCKS));
	sourceGroupInfos[LanguageType::C].push_back(SourceGroupInfo(SourceGroupType::C_EMPTY));
	sourceGroupInfos[LanguageType::CXX].push_back(SourceGroupInfo(SourceGroupType::CXX_CDB, true));
	sourceGroupInfos[LanguageType::CXX].push_back(SourceGroupInfo(SourceGroupType::CXX_VS));
	sourceGroupInfos[LanguageType::CXX].push_back(SourceGroupInfo(SourceGroupType::CXX_CODEBLOCKS));
	sourceGroupInfos[LanguageType::CXX].push_back(SourceGroupInfo(SourceGroupType::CXX_EMPTY));
#endif	  // BUILD_CXX_LANGUAGE_PACKAGE
#if BUILD_JAVA_LANGUAGE_PACKAGE
	sourceGroupInfos[LanguageType::JAVA].push_back(SourceGroupInfo(SourceGroupType::JAVA_MAVEN));
	sourceGroupInfos[LanguageType::JAVA].push_back(SourceGroupInfo(SourceGroupType::JAVA_GRADLE));
	sourceGroupInfos[LanguageType::JAVA].push_back(SourceGroupInfo(SourceGroupType::JAVA_EMPTY));
#endif	  // BUILD_JAVA_LANGUAGE_PACKAGE
#if BUILD_PYTHON_LANGUAGE_PACKAGE
	sourceGroupInfos[LanguageType::PYTHON].push_back(SourceGroupInfo(SourceGroupType::PYTHON_EMPTY));
#endif	  // BUILD_PYTHON_LANGUAGE_PACKAGE
	sourceGroupInfos[LanguageType::CUSTOM].push_back(SourceGroupInfo(SourceGroupType::CUSTOM_COMMAND));

	// define which icons should be used for which kind of source group
#if BUILD_CXX_LANGUAGE_PACKAGE
	m_sourceGroupTypeIconName[SourceGroupType::C_EMPTY]        = QtResources::ICON_EMPTY_ICON;
	m_sourceGroupTypeIconName[SourceGroupType::CXX_EMPTY]      = QtResources::ICON_EMPTY_ICON;
	m_sourceGroupTypeIconName[SourceGroupType::CXX_CDB]        = QtResources::ICON_CDB_ICON;
	m_sourceGroupTypeIconName[SourceGroupType::CXX_VS]         = QtResources::ICON_VS_ICON;
	m_sourceGroupTypeIconName[SourceGroupType::CXX_CODEBLOCKS] = QtResources::ICON_CBP_ICON;
#endif // BUILD_CXX_LANGUAGE_PACKAGE
#if BUILD_JAVA_LANGUAGE_PACKAGE
	m_sourceGroupTypeIconName[SourceGroupType::JAVA_EMPTY]  = QtResources::ICON_EMPTY_ICON;
	m_sourceGroupTypeIconName[SourceGroupType::JAVA_MAVEN]  = QtResources::ICON_MVN_ICON;
	m_sourceGroupTypeIconName[SourceGroupType::JAVA_GRADLE] = QtResources::ICON_GRADLE_ICON;
#endif // BUILD_JAVA_LANGUAGE_PACKAGE
#if BUILD_PYTHON_LANGUAGE_PACKAGE
	m_sourceGroupTypeIconName[SourceGroupType::PYTHON_EMPTY] = QtResources::ICON_EMPTY_ICON;
#endif	  // BUILD_PYTHON_LANGUAGE_PACKAGE
	m_sourceGroupTypeIconName[SourceGroupType::CUSTOM_COMMAND] = QtResources::ICON_EMPTY_ICON;

	// define descriptions for each kind of Source Group
#if BUILD_CXX_LANGUAGE_PACKAGE
	m_sourceGroupTypeDescriptions[SourceGroupType::C_EMPTY] =
		"Create a new Source Group by defining which C files will be indexed.";
	m_sourceGroupTypeDescriptions[SourceGroupType::CXX_EMPTY] =
		"Create a new Source Group by defining which C++ files will be indexed.";
	m_sourceGroupTypeDescriptions[SourceGroupType::CXX_CDB] =
		"Create a Source Group from an existing Compilation Database file (compile_commands.json). "
		"It can be exported from CMake<br />(-DCMAKE_EXPORT_COMPILE_COMMANDS=ON) and Make projects "
		"or from the Qt Creator since version 4.8. Have a look at the "
		"<a href=\"" +
		utility::getDocumentationLink() +
		"#cc-source-group-from-compilation-database\">documentation</a>.";
	m_sourceGroupTypeDescriptions[SourceGroupType::CXX_VS] =
		"<p>Create a new Source Group from an existing Visual Studio Solution file.</p>"
		"<p><b>Note</b>: Requires a running Visual Studio instance with the "
		"<a href=\"" +
		utility::getDocumentationLink() +
		"#visual-studio\">Sourcetrail Visual Studio Extension</a> installed.</p>";
	m_sourceGroupTypeDescriptions[SourceGroupType::CXX_CODEBLOCKS] =
		"<p>Create a new Source Group from an existing Code::Blocks project file.</p>"
		"<p><b>Note</b>: A \".cbp\" file will also get generated by the <b>Qt Creator</b> if a "
		"CMakeLists file is imported.</p>";
#endif	  // BUILD_CXX_LANGUAGE_PACKAGE
#if BUILD_JAVA_LANGUAGE_PACKAGE
	m_sourceGroupTypeDescriptions[SourceGroupType::JAVA_EMPTY] =
		"Create a new Source Group by defining which Java files will be indexed.";
	m_sourceGroupTypeDescriptions[SourceGroupType::JAVA_MAVEN] =
		"Create a new Source Group from an existing Maven project.";
	m_sourceGroupTypeDescriptions[SourceGroupType::JAVA_GRADLE] =
		"Create a new Source Group from an existing Gradle project.";
#endif	  // BUILD_JAVA_LANGUAGE_PACKAGE
#if BUILD_PYTHON_LANGUAGE_PACKAGE
	m_sourceGroupTypeDescriptions[SourceGroupType::PYTHON_EMPTY] =
		"<p>Create a new Source Group by defining which Python files will be indexed. This Source "
		"Group type uses the "
		"<a href=\"https://github.com/petermost/"
		"SourcetrailPythonIndexer\">SourcetrailPythonIndexer</a> " +
		pythonIndexerVersion + "in the background.</p>";
#endif	  // BUILD_PYTHON_LANGUAGE_PACKAGE
	m_sourceGroupTypeDescriptions[SourceGroupType::CUSTOM_COMMAND] =
		"Create a new Source Group executing a custom command on each source file. "
		"This Source Group type can be used on <a "
		"href=\"https://github.com/petermost/SourcetrailDB\">SourcetrailDB</a> binaries that "
		"add "
		"custom language support to Sourcetrail.<br /><br />Current Database Version: " +
		std::to_string(SqliteIndexStorage::getStorageVersion());

	QVBoxLayout* vlayout = new QVBoxLayout();
	vlayout->setContentsMargins(0, 10, 0, 0);
	vlayout->setSpacing(10);

	m_languages = new QButtonGroup();
	for (auto& it: sourceGroupInfos)
	{
		QPushButton* b = new QPushButton(languageTypeToString(it.first).c_str(), this);
		b->setObjectName(QStringLiteral("menuButton"));
		b->setCheckable(true);
		b->setProperty("language_type", QVariant::fromValue(it.first));
		m_languages->addButton(b);
		vlayout->addWidget(b);
	}

	vlayout->addStretch();
	layout->addLayout(vlayout, 0, QtProjectWizardWindow::FRONT_COL, Qt::AlignRight);

	connect(m_languages, qOverload<QAbstractButton*>(&QButtonGroup::buttonClicked), [this](QAbstractButton* button)
	{
		LanguageType selectedLanguage = qt_variant_cast<LanguageType>(button->property("language_type"));

		bool hasRecommended = false;
		for (auto& it: m_buttons)
		{
			it.second->setExclusive(false);
			for (QAbstractButton* button: it.second->buttons())
			{
				button->setChecked(false);
				button->setVisible(it.first == selectedLanguage);

				if (it.first == selectedLanguage)
				{
					hasRecommended = hasRecommended | button->property("recommended").toBool();
				}
			}
			it.second->setExclusive(true);
		}

		m_window->setNextEnabled(false);
		m_title->setText("Source Group Types - " + m_languages->checkedButton()->text());

		m_description->setText(
			hasRecommended ? QStringLiteral("<b>* recommended</b>") : QLatin1String(""));
	});

	QtFlowLayout* flayout = new QtFlowLayout(10, 0, 0);

	for (auto& languageIt: sourceGroupInfos)
	{
		QButtonGroup* sourceGroupButtons = new QButtonGroup(this);

		for (auto& sourceGroupIt: languageIt.second)
		{
			std::string name = sourceGroupTypeToProjectSetupString(sourceGroupIt.type);
			if (sourceGroupIt.recommended)
			{
				name += "*";
			}

			QToolButton* b = createSourceGroupButton(
				utility::insertLineBreaksAtBlankSpaces(name, 15).c_str(), 
				QString::fromStdString(m_sourceGroupTypeIconName[sourceGroupIt.type]));

			if (sourceGroupIt.recommended)
			{
				b->setStyleSheet(QStringLiteral("font-weight: bold"));
			}

			b->setProperty("source_group_type", int(sourceGroupIt.type));
			b->setProperty("recommended", sourceGroupIt.recommended);
			sourceGroupButtons->addButton(b);
			flayout->addWidget(b);
		}

		m_buttons[languageIt.first] = sourceGroupButtons;
	}

	for (auto& it: m_buttons)
	{
		connect(
			it.second,
			qOverload<QAbstractButton*>(&QButtonGroup::buttonClicked),
			[this](QAbstractButton* button)
			{
				SourceGroupType selectedType = SourceGroupType::UNKNOWN;
				bool ok = false;
				int selectedTypeInt = button->property("source_group_type").toInt(&ok);
				if (ok)
				{
					selectedType = SourceGroupType(selectedTypeInt);
				}

				m_description->setText(m_sourceGroupTypeDescriptions[selectedType].c_str());

				m_window->setNextEnabled(true);
			});
	}

	QWidget* container = new QWidget();
	QVBoxLayout* containerLayout = new QVBoxLayout();
	containerLayout->setContentsMargins(0, 0, 0, 0);

	QFrame* groupContainer = new QFrame();
	groupContainer->setObjectName(QStringLiteral("sourceGroupContainer"));
	groupContainer->setLayout(flayout);
	containerLayout->addWidget(groupContainer, 0);

	m_description = new QLabel(QStringLiteral(" \n \n"));
	m_description->setWordWrap(true);
	m_description->setOpenExternalLinks(true);
	m_description->setObjectName(QStringLiteral("sourceGroupDescription"));
	m_description->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
	m_description->setMinimumHeight(80);
	containerLayout->addWidget(m_description, 0);

	container->setLayout(containerLayout);
	layout->addWidget(container, 0, QtProjectWizardWindow::BACK_COL);

	m_title = new QLabel(QStringLiteral("Source Group Types"));
	m_title->setObjectName(QStringLiteral("sourceGroupTitle"));

	layout->addWidget(m_title, 0, QtProjectWizardWindow::BACK_COL, Qt::AlignLeft | Qt::AlignTop);

	layout->setRowStretch(0, 0);
	layout->setColumnStretch(QtProjectWizardWindow::FRONT_COL, 0);
	layout->setColumnStretch(QtProjectWizardWindow::BACK_COL, 1);
	layout->setHorizontalSpacing(0);

	m_languages->buttons().constFirst()->click();
}

void QtProjectWizardContentSelect::save()
{
	SourceGroupType selectedType = SourceGroupType::UNKNOWN;

	for (auto& it: m_buttons)
	{
		if (QAbstractButton* b = it.second->checkedButton())
		{
			bool ok = false;
			int selectedTypeInt = b->property("source_group_type").toInt(&ok);
			if (ok)
			{
				selectedType = SourceGroupType(selectedTypeInt);
				break;
			}
		}
	}
	emit selected(selectedType);
}

bool QtProjectWizardContentSelect::check()
{
	bool sourceGroupChosen = false;

	for (auto& it: m_buttons)
	{
		if (it.second->checkedId() != -1)
		{
			sourceGroupChosen = true;
			break;
		}
	}

	if (!sourceGroupChosen)
	{
		QtMessageBox msgBox(m_window);
		msgBox.setText(QStringLiteral("Please choose a method of creating a new Source Group."));
		msgBox.execModal();
		return false;
	}

	return true;
}
