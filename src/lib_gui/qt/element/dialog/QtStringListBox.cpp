#include "QtStringListBox.h"

#include <QListWidget>

#include "QtStringListBoxItem.h"

QtStringListBox::QtStringListBox(QWidget* parent, const QString& listName)
	: QtListBox(parent, listName)
{
}

std::vector<std::string> QtStringListBox::getStrings()
{
	std::vector<std::string> strings;
	for (int i = 0; i < m_list->count(); ++i)
	{
		QtListBoxItem* widget = dynamic_cast<QtListBoxItem*>(m_list->itemWidget(m_list->item(i)));
		strings.push_back(widget->getText().toStdString());
	}
	return strings;
}

void QtStringListBox::setStrings(const std::vector<std::string>& strings, bool readOnly)
{
	clear();

	for (const std::string& str: strings)
	{
		QtListBoxItem* itemWidget = addListBoxItemWithText(QString::fromStdString(str));
		itemWidget->setReadOnly(readOnly);
	}
}

QtListBoxItem* QtStringListBox::createListBoxItem(QListWidgetItem* item)
{
	return new QtStringListBoxItem(this, item);
}
