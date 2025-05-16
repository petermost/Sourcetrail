#ifndef QT_GRAPH_NODE_BUNDLE_H
#define QT_GRAPH_NODE_BUNDLE_H

#include "Node.h"
#include "QtGraphNode.h"

class QtCountCircleItem;

class QtGraphNodeBundle: public QtGraphNode
{
	Q_OBJECT
public:
	QtGraphNodeBundle(
		GraphFocusHandler* focusHandler,
		Id tokenId,
		size_t nodeCount,
		NodeType type,
		const std::string& name,
		bool interactive);
	~QtGraphNodeBundle() override;

	// QtGraphNode implementation
	bool isBundleNode() const override;
	Id getTokenId() const override;

	void onClick() override;
	void updateStyle() override;

protected:
	void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

private:
	QtCountCircleItem* m_circle;
	Id m_tokenId;
	NodeType m_type;
};

#endif	  // QT_GRAPH_NODE_BUNDLE_H
