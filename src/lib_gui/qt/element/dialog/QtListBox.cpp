#include "QtListBox.h"

#include "QtIconButton.h"
#include "QtResources.h"
#include "QtStringListBoxItem.h"
#include "QtTextEditDialog.h"
#include "utilityQt.h"
#include "utilityString.h"

#include <QBoxLayout>
#include <QLabel>
#include <QScrollBar>

using namespace utility;

void QtListWidget::mouseDoubleClickEvent(QMouseEvent*  /*event*/)
{
	QModelIndex index;
	emit doubleClicked(index);
}

void QtListWidget::wheelEvent(QWheelEvent* event)
{
	QScrollBar* bar = verticalScrollBar();
	bool down = event->angleDelta().y() < 0;

	if (bar->minimum() == bar->maximum() || (down && bar->value() == bar->maximum()) ||
		(!down && bar->value() == bar->minimum()))
	{
		event->ignore();
	}
	else
	{
		QListWidget::wheelEvent(event);
	}
}

QtListBox::QtListBox(QWidget *parent, const QString &listName)
	: QFrame(parent)
	, m_listName(listName)
{
	QBoxLayout *layout = new QVBoxLayout();
	layout->setSpacing(0);
	layout->setContentsMargins(0, 6, 0, 0);
	layout->setAlignment(Qt::AlignTop);

	m_list = new QtListWidget(this);
	m_list->setObjectName(QStringLiteral("list"));
	m_list->setAttribute(Qt::WA_MacShowFocusRect, false);
	connect(m_list, &QListWidget::doubleClicked, this, &QtListBox::doubleClicked);

	setStyleSheet(QtResources::loadStyleSheet(QtResources::WINDOW_LISTBOX_CSS));
	layout->addWidget(m_list, 5);

	QWidget *buttonContainer = new QWidget(this);
	buttonContainer->setObjectName(QStringLiteral("bar"));

	QHBoxLayout *barLayout = new QHBoxLayout();
	barLayout->setContentsMargins(8, 4, 8, 2);
	barLayout->setSpacing(0);

	m_addButton = new QtIconButton(QtResources::WINDOW_PLUS, QtResources::WINDOW_PLUS_HOVER);
	m_addButton->setIconSize(QSize(16, 16));
	m_addButton->setObjectName(QStringLiteral("plusButton"));
	m_addButton->setToolTip(tr("add line"));
	barLayout->addWidget(m_addButton);

	m_removeButton = new QtIconButton(QtResources::WINDOW_MINUS, QtResources::WINDOW_MINUS_HOVER);
	m_removeButton->setIconSize(QSize(16, 16));
	m_removeButton->setObjectName(QStringLiteral("minusButton"));
	m_removeButton->setToolTip(tr("remove line"));
	barLayout->addWidget(m_removeButton);

	barLayout->addStretch();

	m_innerBarLayout = new QHBoxLayout();
	barLayout->addLayout(m_innerBarLayout);

	QPushButton *editButton = new QtIconButton(QtResources::CODE_VIEW_EDIT, FilePath());
	editButton->setIconSize(QSize(16, 16));
	editButton->setObjectName(QStringLiteral("editButton"));
	editButton->setToolTip(tr("edit plain text"));
	barLayout->addWidget(editButton);

	buttonContainer->setLayout(barLayout);
	layout->addWidget(buttonContainer, 0);
	setLayout(layout);

	connect(m_addButton, &QPushButton::clicked, this, &QtListBox::addListBoxItem);
	connect(m_removeButton, &QPushButton::clicked, this, &QtListBox::removeListBoxItem);
	connect(editButton, &QPushButton::clicked, this, &QtListBox::showEditDialog);

	setAcceptDrops(true);
	setMaximumHeight(160);
}

void QtListBox::clear()
{
	m_list->clear();
}

void QtListBox::addWidgetToBar(QWidget* widget)
{
	if (m_innerBarLayout)
	{
		m_innerBarLayout->addWidget(widget);
	}
}

QtListBoxItem* QtListBox::addListBoxItemWithText(const QString& text)
{
	QtListBoxItem* item = addListBoxItem();
	item->setText(text);
	return item;
}

void QtListBox::selectItem(QListWidgetItem* item)
{
	for (int i = 0; i < m_list->count(); i++)
	{
		m_list->item(i)->setSelected(false);
	}

	item->setSelected(true);
}

QtListBoxItem* QtListBox::addListBoxItem()
{
	QListWidgetItem* item = new QListWidgetItem(m_list);
	m_list->addItem(item);

	QtListBoxItem* widget = createListBoxItem(item);
	m_list->setItemWidget(item, widget);

	m_list->scrollToItem(item);
	widget->setFocus();

	return widget;
}

void QtListBox::removeListBoxItem()
{
	if (m_list->selectedItems().empty())
	{
		return;
	}

	int rowIndex = m_list->row(m_list->selectedItems().first());

	qDeleteAll(m_list->selectedItems());

	if (rowIndex == m_list->count())
	{
		rowIndex -= 1;
	}

	if (rowIndex >= 0)
	{
		m_list->setCurrentRow(rowIndex);
	}
}

void QtListBox::showEditDialog()
{
	if (!m_editDialog)
	{
		m_editDialog = std::make_shared<QtTextEditDialog>(
			m_listName, "Edit the list in plain text. Each line is one item.");
		m_editDialog->setup();

		std::vector<std::string> list;
		for (int i = 0; i < m_list->count(); ++i)
		{
			QtListBoxItem* item = dynamic_cast<QtListBoxItem*>(m_list->itemWidget(m_list->item(i)));
			if (!item->getReadOnly())
			{
				list.push_back(item->getText().toStdString());
			}
		}

		m_editDialog->setText(utility::join(list, "\n"));

		connect(
			m_editDialog.get(), &QtTextEditDialog::canceled, this, &QtListBox::canceledEditDialog);
		connect(m_editDialog.get(), &QtTextEditDialog::finished, this, &QtListBox::savedEditDialog);
	}

	m_editDialog->show();
	m_editDialog->raise();
}

void QtListBox::canceledEditDialog()
{
	m_editDialog->hide();
	m_editDialog.reset();

	window()->raise();
}

void QtListBox::savedEditDialog()
{
	std::vector<std::string> readOnlyLines;
	for (int i = 0; i < m_list->count(); ++i)
	{
		QtListBoxItem* item = dynamic_cast<QtListBoxItem*>(m_list->itemWidget(m_list->item(i)));
		if (item->getReadOnly())
		{
			readOnlyLines.push_back(item->getText().toStdString());
		}
	}

	std::vector<std::string> lines = utility::splitToVector(m_editDialog->getText(), "\n");
	for (size_t i = 0; i < lines.size(); i++)
	{
		lines[i] = utility::trim(lines[i]);

		if (lines[i].empty())
		{
			lines.erase(lines.begin() + i);
			i--;
		}
	}

	clear();

	for (const std::string& line: readOnlyLines)
	{
		QtListBoxItem* itemWidget = addListBoxItemWithText(QString::fromStdString(line));
		itemWidget->setReadOnly(true);
	}

	for (const std::string& line: lines)
	{
		QtListBoxItem* itemWidget = addListBoxItemWithText(QString::fromStdString(line));
		itemWidget->setReadOnly(false);
	}

	canceledEditDialog();
}

void QtListBox::doubleClicked(const QModelIndex& index)
{
	if (!index.isValid())
	{
		addListBoxItem();
	}
}
