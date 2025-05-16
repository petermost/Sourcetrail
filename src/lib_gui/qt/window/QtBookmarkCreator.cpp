#include "QtBookmarkCreator.h"

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>

#include "MessageStatus.h"

QtBookmarkCreator::QtBookmarkCreator(ControllerProxy<BookmarkController>* controllerProxy, QWidget* parent, BookmarkId bookmarkId)
	: QtBookmarkWindow(controllerProxy, false, parent)
	, m_editBookmarkId(bookmarkId)
	, m_nodeId(0)
{
	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	{
		// title
		QLabel* title = new QLabel(m_editBookmarkId != BookmarkId::NONE ? tr("Edit Bookmark") : tr("Create Bookmark"));
		title->setObjectName(QStringLiteral("creator_title_label"));
		mainLayout->addWidget(title);
	}

	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(20, 10, 20, 20);

	{
		// name
		QLabel* nameLabel = new QLabel(tr("Name"));
		nameLabel->setObjectName(QStringLiteral("creator_label"));
		layout->addWidget(nameLabel);

		m_displayName = new QLineEdit();
		m_displayName->setObjectName(QStringLiteral("creator_name_edit"));
		m_displayName->setPlaceholderText(tr("Name"));
		m_displayName->setAttribute(Qt::WA_MacShowFocusRect, false);
		layout->addWidget(m_displayName);

		connect(m_displayName, &QLineEdit::textChanged, this, &QtBookmarkCreator::onNameChanged);

		layout->addSpacing(15);
	}

	{
		// comment
		QLabel* commentLabel = new QLabel(tr("Comment"));
		commentLabel->setObjectName(QStringLiteral("creator_label"));
		layout->addWidget(commentLabel);

		m_commentBox = new QTextEdit();
		m_commentBox->setObjectName(QStringLiteral("creator_comment_box"));
		m_commentBox->setPlaceholderText(tr("Comment"));
		layout->addWidget(m_commentBox);

		layout->addSpacing(15);
	}

	{
		// category
		QLabel* categoryLabel = new QLabel(tr("Choose or Create Category"));
		categoryLabel->setObjectName(QStringLiteral("creator_label"));
		layout->addWidget(categoryLabel);

		m_categoryBox = new QComboBox();
		m_categoryBox->setObjectName(QStringLiteral("creator_category_box"));
		m_categoryBox->addItem(QLatin1String(""));
		m_categoryBox->setEditable(true);
		m_categoryBox->lineEdit()->setPlaceholderText(tr("Category"));
		m_categoryBox->setInsertPolicy(QComboBox::InsertPolicy::InsertAtTop);
		layout->addWidget(m_categoryBox);

		layout->addSpacing(20);
	}

	{
		layout->addLayout(createButtons());
		setPreviousVisible(false);

		updateNextButton(m_editBookmarkId != BookmarkId::NONE ? tr("Save") : tr("Create"));
		setNextDefault(true);
	}

	mainLayout->addLayout(layout);

	refreshStyleSheet();
}

void QtBookmarkCreator::setDisplayName(const std::string& name)
{
	m_displayName->setText(QString::fromStdString(name));
}

void QtBookmarkCreator::setComment(const std::string& comment)
{
	m_commentBox->setText(QString::fromStdString(comment));
}

void QtBookmarkCreator::setBookmarkCategories(const std::vector<BookmarkCategory>& categories)
{
	for (unsigned int i = 0; i < categories.size(); i++)
	{
		m_categoryBox->addItem(QString::fromStdString(categories[i].getName()));
	}
}

void QtBookmarkCreator::setCurrentBookmarkCategory(const BookmarkCategory& category)
{
	int index = m_categoryBox->findText(QString::fromStdString(category.getName()));

	if (index > -1)
	{
		m_categoryBox->setCurrentIndex(index);
	}
	else
	{
		m_categoryBox->addItem(QString::fromStdString(category.getName()));
		m_categoryBox->setCurrentIndex(1);
	}
}

void QtBookmarkCreator::setNodeId(Id nodeId)
{
	m_nodeId = nodeId;
}

void QtBookmarkCreator::handleNext()
{
	std::string name = m_displayName->text().toStdString();
	std::string comment = m_commentBox->toPlainText().toStdString();
	std::string category = m_categoryBox->currentText().toStdString();

	if (m_editBookmarkId != BookmarkId::NONE)
	{
		m_controllerProxy->executeAsTaskWithArgs(
			&BookmarkController::editBookmark, m_editBookmarkId, name, comment, category);
	}
	else
	{
		m_controllerProxy->executeAsTaskWithArgs(
			&BookmarkController::createBookmark, name, comment, category, m_nodeId);

		MessageStatus("Creating Bookmark for active Token").dispatch();
	}

	close();
}

void QtBookmarkCreator::handleClose()
{
	close();
}

void QtBookmarkCreator::onNameChanged(const QString& text)
{
	setNextEnabled(text.length() > 0);
}
