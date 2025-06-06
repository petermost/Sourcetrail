#include "QtListBoxItem.h"

#include "QtFileDialog.h"
#include "QtLineEdit.h"
#include "QtListBox.h"

#include <QBoxLayout>
#include <QListWidget>
#include <QPushButton>

QtListBoxItem::QtListBoxItem(QListWidgetItem* item, QWidget* parent)
	: QWidget(parent), m_item(item)
{
	QBoxLayout* layout = new QHBoxLayout();
	layout->setSpacing(3);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setAlignment(Qt::AlignTop);
	setLayout(layout);

	m_data = new QtLineEdit(this);
	m_data->setAttribute(Qt::WA_MacShowFocusRect, false);
	m_data->setAttribute(Qt::WA_LayoutUsesWidgetRect);	  // fixes layouting on Mac
	m_data->setObjectName(QStringLiteral("field"));
	m_data->setAcceptDrops(false);
	layout->addWidget(m_data);

	connect(m_data, &QtLineEdit::focus, this, &QtListBoxItem::handleFocus);
}

QString QtListBoxItem::getText() const
{
	return m_data->text();
}

void QtListBoxItem::setText(const QString& text)
{
	m_data->setText(text);
}

bool QtListBoxItem::getReadOnly() const
{
	return m_readOnly;
}

void QtListBoxItem::setReadOnly(bool readOnly)
{
	if (readOnly != m_readOnly)
	{
		m_readOnly = readOnly;

		if (readOnly)
		{
			m_item->setFlags(m_item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
		}
		else
		{
			m_item->setFlags(m_item->flags() | ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
		}

		m_data->setReadOnly(readOnly);
		m_data->setEnabled(!readOnly);

		onReadOnlyChanged();
	}
}

void QtListBoxItem::setFocus()
{
	m_data->setFocus(Qt::OtherFocusReason);
}

void QtListBoxItem::selectItem()
{
	getListBox()->selectItem(m_item);
}

void QtListBoxItem::handleFocus()
{
	selectItem();
}

void QtListBoxItem::onReadOnlyChanged() {}
