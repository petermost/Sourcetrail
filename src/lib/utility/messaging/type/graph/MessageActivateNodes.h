#ifndef MESSAGE_ACTIVATE_NODES_H
#define MESSAGE_ACTIVATE_NODES_H

#include "Message.h"
#include "TabIds.h"
#include "types.h"

class MessageActivateNodes: public Message<MessageActivateNodes>
{
public:
	struct ActiveNode
	{
		ActiveNode(): nodeId(0), nameHierarchy(NAME_DELIMITER_UNKNOWN) {}
		Id nodeId;
		NameHierarchy nameHierarchy;
	};

	MessageActivateNodes(Id tokenId = 0)
	{
		if (tokenId > 0)
		{
			addNode(tokenId);
		}

		setSchedulerId(TabIds::currentTab());
	}

	void addNode(Id tokenId)
	{
		ActiveNode node;
		node.nodeId = tokenId;
		nodes.push_back(node);
	}

	void addNode(Id tokenId, const NameHierarchy& nameHierarchy)
	{
		ActiveNode node;
		node.nodeId = tokenId;
		node.nameHierarchy = nameHierarchy;
		nodes.push_back(node);
	}

	static const std::string getStaticType()
	{
		return "MessageActivateNodes";
	}

	void print(std::ostream& os) const override
	{
		for (const ActiveNode& node: nodes)
		{
			os << node.nodeId << " ";
		}
	}

	std::vector<ActiveNode> nodes;
};

#endif	  // MESSAGE_ACTIVATE_NODES_H
