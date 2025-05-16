#ifndef MESSAGE_GRAPH_NODE_BUNDLE_SPLIT_H
#define MESSAGE_GRAPH_NODE_BUNDLE_SPLIT_H

#include "Message.h"
#include "TabIds.h"
#include "types.h"

class MessageGraphNodeBundleSplit: public Message<MessageGraphNodeBundleSplit>
{
public:
	MessageGraphNodeBundleSplit(Id bundleId, bool removeOtherNodes = false, bool layoutToList = false)
		: bundleId(bundleId), removeOtherNodes(removeOtherNodes), layoutToList(layoutToList)
	{
		setSchedulerId(TabIds::currentTab());
	}

	static const std::string getStaticType()
	{
		return "MessageGraphNodeBundleSplit";
	}

	void print(std::ostream& os) const override
	{
		os << bundleId;
	}

	Id bundleId;
	bool removeOtherNodes;
	bool layoutToList;
};

#endif	  // MESSAGE_GRAPH_NODE_BUNDLE_SPLIT_H
