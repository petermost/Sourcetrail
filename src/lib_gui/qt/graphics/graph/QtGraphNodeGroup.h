#ifndef QT_GRAPH_NODE_GROUP_H
#define QT_GRAPH_NODE_GROUP_H

#include "GroupType.h"
#include "QtGraphNode.h"

class QGraphicsPolygonItem;
class QPainterPath;

class QtGraphNodeGroup: public QtGraphNode
{
	Q_OBJECT
public:
	QtGraphNodeGroup(
		GraphFocusHandler* focusHandler,
		Id tokenId,
		const std::string& name,
		GroupType type,
		bool interactive);
	~QtGraphNodeGroup() override;

	// QtGraphNode implementation
	bool isGroupNode() const override;
	Id getTokenId() const override;

	void onClick() override;
	void updateStyle() override;

	QPainterPath shape() const override;

protected:
	void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
	void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;

private:
	Id m_tokenId;
	GroupType m_type;

	QGraphicsPolygonItem* m_background = nullptr;
	mutable QPainterPath m_path;
};

#endif	  // QT_GRAPH_NODE_GROUP_H
