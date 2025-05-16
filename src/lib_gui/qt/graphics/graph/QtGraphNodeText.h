#ifndef QT_GRAPH_NODE_TEXT_H
#define QT_GRAPH_NODE_TEXT_H

#include "QtGraphNode.h"

class QtGraphNodeText: public QtGraphNode
{
	Q_OBJECT
public:
	QtGraphNodeText(const std::string& name, int fontSizeDiff);
	~QtGraphNodeText() override;

	// QtGraphNode implementation
	bool isTextNode() const override;

	void updateStyle() override;

private:
	int m_fontSizeDiff;
};

#endif	  // QT_GRAPH_NODE_TEXT_H
