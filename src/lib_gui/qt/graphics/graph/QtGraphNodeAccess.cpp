#include "QtGraphNodeAccess.h"

#include "GraphViewStyle.h"
#include "QtDeviceScaledPixmap.h"
#include "TokenComponentAccess.h"
#include "utilityQt.h"
#include "QtResources.h"

#include <QBrush>
#include <QCursor>
#include <QFontMetrics>
#include <QPen>

QtGraphNodeAccess::QtGraphNodeAccess(AccessKind accessKind)
	:  m_accessKind(accessKind)
{
	std::string accessString = TokenComponentAccess::getAccessString(m_accessKind);
	this->setName(accessString);
	this->setCursor(Qt::ArrowCursor);
	m_text->hide();

	std::string iconFileName;
	switch (m_accessKind)
	{
	case AccessKind::PUBLIC:
		iconFileName = QtResources::GRAPH_VIEW_PUBLIC;
		break;
	case AccessKind::PROTECTED:
		iconFileName = QtResources::GRAPH_VIEW_PROTECTED;
		break;
	case AccessKind::PRIVATE:
		iconFileName = QtResources::GRAPH_VIEW_PRIVATE;
		break;
	case AccessKind::DEFAULT:
		iconFileName = QtResources::GRAPH_VIEW_DEFAULT;
		break;
	case AccessKind::TEMPLATE_PARAMETER:
	case AccessKind::TYPE_PARAMETER:
		iconFileName = QtResources::GRAPH_VIEW_TEMPLATE;
		break;
	default:
		break;
	}

	if (iconFileName.size() > 0)
	{
		QtDeviceScaledPixmap pixmap(QString::fromStdString(iconFileName));
		pixmap.scaleToHeight(m_accessIconSize);

		m_accessIcon = new QGraphicsPixmapItem(pixmap.pixmap(), this);
		m_accessIcon->setTransformationMode(Qt::SmoothTransformation);
		m_accessIcon->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
	}
}

QtGraphNodeAccess::~QtGraphNodeAccess() = default;

AccessKind QtGraphNodeAccess::getAccessKind() const
{
	return m_accessKind;
}

bool QtGraphNodeAccess::isAccessNode() const
{
	return true;
}

void QtGraphNodeAccess::addSubNode(QtGraphNode* node)
{
	QtGraphNode::addSubNode(node);
	m_text->show();
}

void QtGraphNodeAccess::updateStyle()
{
	GraphViewStyle::NodeStyle style = GraphViewStyle::getStyleOfAccessNode();
	setStyle(style);

	QFont font = m_text->font();
	font.setCapitalization(QFont::AllUppercase);
	m_text->setFont(font);

	if (m_accessIcon)
	{
		m_text->setPos(
			static_cast<qreal>(style.textOffset.x + m_accessIconSize + 3),
			static_cast<qreal>(style.textOffset.y + m_accessIconSize - style.fontSize));
		m_accessIcon->setPos(style.textOffset.x, style.textOffset.y);

		m_accessIcon->setPixmap(
			utility::colorizePixmap(m_accessIcon->pixmap(), style.color.icon.c_str()));
	}
	else
	{
		m_text->setPos(
			static_cast<qreal>(style.textOffset.x),
			static_cast<qreal>(style.textOffset.y + m_accessIconSize + 2 - style.fontSize));
	}
}

void QtGraphNodeAccess::hideLabel()
{
	m_text->hide();
}
