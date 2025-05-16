#include "QtPathListBoxItem.h"

#include "QtFileDialog.h"
#include "QtIconButton.h"
#include "QtPathListBox.h"
#include "QtResources.h"

#include <QBoxLayout>
#include <QListWidget>
#include <QPushButton>

QtPathListBoxItem::QtPathListBoxItem(QtPathListBox* listBox, QListWidgetItem* item, QWidget* parent)
	: QtListBoxItem(item, parent), m_listBox(listBox)
{
	m_button = new QtIconButton(QtResources::WINDOW_DOTS, QtResources::WINDOW_DOTS_HOVER);
	m_button->setIconSize(QSize(16, 16));
	m_button->setObjectName(QStringLiteral("dotsButton"));
	layout()->addWidget(m_button);

	connect(m_button, &QPushButton::clicked, this, &QtPathListBoxItem::handleButtonPress);
}

QtListBox* QtPathListBoxItem::getListBox()
{
	return m_listBox;
}

void QtPathListBoxItem::handleButtonPress()
{
	FilePath path(getText().toStdString());
	m_listBox->makeAbsolute(path);

	QStringList list;
	switch (m_listBox->getSelectionPolicy())
	{
	case QtPathListBox::SELECTION_POLICY_FILES_ONLY:
		list.append(QtFileDialog::getOpenFileName(
			this, QStringLiteral("Select Directory"), path, QLatin1String("")));
		break;
	case QtPathListBox::SELECTION_POLICY_DIRECTORIES_ONLY:
		list.append(
			QtFileDialog::getExistingDirectory(this, QStringLiteral("Select Directory"), path));
		break;
	case QtPathListBox::SELECTION_POLICY_FILES_AND_DIRECTORIES:
		list = QtFileDialog::getFileNamesAndDirectories(this, path);
		break;
	}

	if (!list.isEmpty())
	{
		FilePath path(list.at(0).toStdString());
		m_listBox->makeRelativeIfShorter(path);
		setText(QString::fromStdString(path.str()));
	}

	for (int i = 1; i < list.size(); i++)
	{
		FilePath path(list.at(i).toStdString());
		m_listBox->makeRelativeIfShorter(path);
		getListBox()->addListBoxItemWithText(QString::fromStdString(path.str()));
	}

	selectItem();
}

void QtPathListBoxItem::onReadOnlyChanged()
{
	m_button->setEnabled(!getReadOnly());
}
