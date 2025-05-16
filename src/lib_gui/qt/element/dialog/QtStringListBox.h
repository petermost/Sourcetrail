#ifndef QT_STRING_LIST_BOX_H
#define QT_STRING_LIST_BOX_H

#include "QtListBox.h"
#include "QtStringListBoxItem.h"

class QtStringListBox: public QtListBox
{
public:
	QtStringListBox(QWidget* parent, const QString& listName);

	std::vector<std::string> getStrings();
	void setStrings(const std::vector<std::string>& strings, bool readOnly = false);

private:
	QtListBoxItem* createListBoxItem(QListWidgetItem* item) override;
};

#endif	  // QT_STRING_LIST_BOX_H
