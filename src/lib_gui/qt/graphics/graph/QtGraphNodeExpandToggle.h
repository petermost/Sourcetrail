#ifndef QT_EXPAND_TOGGLE_H
#define QT_EXPAND_TOGGLE_H

#include <QGraphicsItem>

#include "QtGraphNode.h"

class QtGraphNodeExpandToggle: public QtGraphNode
{
	Q_OBJECT
public:
	QtGraphNodeExpandToggle(bool expanded, int invisibleSubNodeCount);
	~QtGraphNodeExpandToggle() override;

	// QtGraphNode implementation
	bool isExpandToggleNode() const override;

	void onClick() override;
	void updateStyle() override;

	bool isExpanded() const;

protected:
	void matchName(const std::string&  /*query*/, std::vector<QtGraphNode*>*  /*matchedNodes*/) override {}

private:
	QGraphicsPixmapItem* m_icon;
	bool m_invisibleSubNodeCount;
	bool m_expanded;
};

#endif	  // QT_EXPAND_TOGGLE_H
