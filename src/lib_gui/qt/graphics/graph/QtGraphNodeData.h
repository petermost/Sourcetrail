#ifndef QT_GRAPH_NODE_DATA_H
#define QT_GRAPH_NODE_DATA_H

#include "QtGraphNode.h"

class FilePath;

class QtGraphNodeData: public QtGraphNode
{
	Q_OBJECT
public:
	QtGraphNodeData(
		GraphFocusHandler* focusHandler,
		const Node* data,
		const std::string& name,
		bool childVisible,
		bool hasQualifier,
		bool isInteractive);
	~QtGraphNodeData() override;

	const Node* getData() const;
	FilePath getFilePath() const;

	// QtGraphNode implementation
	bool isDataNode() const override;

	Id getTokenId() const override;

	void onClick() override;
	void onMiddleClick() override;
	void updateStyle() override;

protected:
	void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

private:
	const Node* m_data;
	bool m_childVisible;
	bool m_hasQualifier;
};

#endif	  // QT_GRAPH_NODE_DATA_H
