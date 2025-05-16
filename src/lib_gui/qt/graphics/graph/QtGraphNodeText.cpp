#include "QtGraphNodeText.h"

#include <QCursor>

QtGraphNodeText::QtGraphNodeText(const std::string& name, int fontSizeDiff)
	: m_fontSizeDiff(fontSizeDiff)
{
	this->setCursor(Qt::ArrowCursor);

	setName(name);
}

QtGraphNodeText::~QtGraphNodeText() = default;

bool QtGraphNodeText::isTextNode() const
{
	return true;
}

void QtGraphNodeText::updateStyle()
{
	setStyle(GraphViewStyle::getStyleOfTextNode(m_fontSizeDiff));
}
