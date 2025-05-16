#ifndef QT_GRAPH_NODE_ACCESS_H
#define QT_GRAPH_NODE_ACCESS_H

#include "QtGraphNode.h"

#include "AccessKind.h"

class QtGraphNodeAccess: public QtGraphNode
{
	Q_OBJECT
public:
	QtGraphNodeAccess(AccessKind accessKind);
	~QtGraphNodeAccess() override;

	AccessKind getAccessKind() const;

	// QtGraphNode implementation
	bool isAccessNode() const override;

	void addSubNode(QtGraphNode* node) override;
	void updateStyle() override;

	void hideLabel();

protected:
	void matchName(const std::string&  /*query*/, std::vector<QtGraphNode*>*  /*matchedNodes*/) override
	{
	}

private:
	AccessKind m_accessKind;

	QGraphicsPixmapItem* m_accessIcon = nullptr;
	int m_accessIconSize = 16;
};

#endif	  // QT_GRAPH_NODE_ACCESS_H
