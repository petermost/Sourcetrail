#ifndef QT_GRAPH_NODE_QUALIFIER_H
#define QT_GRAPH_NODE_QUALIFIER_H

#include <QGraphicsPolygonItem>
#include <QGraphicsRectItem>

#include "QtGraphNode.h"

class QtGraphNodeQualifier: public QtGraphNode
{
	Q_OBJECT
public:
	QtGraphNodeQualifier(const NameHierarchy& name);
	~QtGraphNodeQualifier() override;

	// QtGraphNode implementation
	bool isQualifierNode() const override;

	bool setPosition(const Vec2i& pos) override;

	void onClick() override;
	void updateStyle() override;

protected:
	void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

	void matchName(const std::string&  /*query*/, std::vector<QtGraphNode*>*  /*matchedNodes*/) override {}

private:
	const NameHierarchy m_qualifierName;

	QGraphicsRectItem* m_background;
	QGraphicsRectItem* m_leftBorder;
	QGraphicsPolygonItem* m_rightArrow;
	QGraphicsPolygonItem* m_rightArrowSmall;
	QGraphicsSimpleTextItem* m_name;

	Vec2i m_pos;
};

#endif	  // QT_GRAPH_NODE_QUALIFIER_H
