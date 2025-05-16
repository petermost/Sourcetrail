#ifndef MESSAGE_GRAPH_NODE_EXPAND_H
#define MESSAGE_GRAPH_NODE_EXPAND_H

#include "Message.h"
#include "TabIds.h"
#include "types.h"

class MessageGraphNodeExpand: public Message<MessageGraphNodeExpand>
{
public:
	MessageGraphNodeExpand(Id tokenId, bool expand, bool ignoreIfNotReplayed = false)
		: tokenId(tokenId), expand(expand), ignoreIfNotReplayed(ignoreIfNotReplayed)
	{
		setSchedulerId(TabIds::currentTab());
	}

	static const std::string getStaticType()
	{
		return "MessageGraphNodeExpand";
	}

	void print(std::ostream& os) const override
	{
		os << tokenId << " ";
		if (expand)
		{
			os << "expand";
		}
		else
		{
			os << "collapse";
		}
	}

	const Id tokenId;
	const bool expand;
	const bool ignoreIfNotReplayed;
};

#endif	  // MESSAGE_GRAPH_NODE_EXPAND_H
