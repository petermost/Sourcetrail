#ifndef MESSAGE_GRAPH_NODE_HIDE_H
#define MESSAGE_GRAPH_NODE_HIDE_H

#include "Message.h"
#include "TabIds.h"
#include "types.h"

class MessageGraphNodeHide: public Message<MessageGraphNodeHide>
{
public:
	MessageGraphNodeHide(Id tokenId): tokenId(tokenId)
	{
		setSchedulerId(TabIds::currentTab());
	}

	static const std::string getStaticType()
	{
		return "MessageGraphNodeHide";
	}

	void print(std::ostream& os) const override
	{
		os << tokenId;
	}

	const Id tokenId;
};

#endif	  // MESSAGE_GRAPH_NODE_HIDE_H
