#ifndef MESSAGE_ACTIVATE_TRAIL_EDGE_H
#define MESSAGE_ACTIVATE_TRAIL_EDGE_H

#include "Edge.h"
#include "NameHierarchy.h"

#include "Message.h"
#include "TabIds.h"

class MessageActivateTrailEdge: public Message<MessageActivateTrailEdge>
{
public:
	MessageActivateTrailEdge(
		const std::vector<Id>& edgeIds,
		Edge::EdgeType type,
		const NameHierarchy& sourceNameHierarchy,
		const NameHierarchy& targetNameHierarchy)
		: edgeIds(edgeIds)
		, type(type)
		, sourceNameHierarchy(sourceNameHierarchy)
		, targetNameHierarchy(targetNameHierarchy)
	{
		setSchedulerId(TabIds::currentTab());
	}

	static const std::string getStaticType()
	{
		return "MessageActivateTrailEdge";
	}

	std::string getFullName() const
	{
		std::string name = Edge::getReadableTypeString(type) + ":";
		name += sourceNameHierarchy.getQualifiedNameWithSignature() + "->";
		name += targetNameHierarchy.getQualifiedNameWithSignature();
		return name;
	}

	void print(std::ostream& os) const override
	{
		for (Id edgeId: edgeIds)
		{
			os << edgeId << ",";
		}
		os << " - " << getFullName();
	}

	const std::vector<Id> edgeIds;
	const Edge::EdgeType type;
	const NameHierarchy sourceNameHierarchy;
	const NameHierarchy targetNameHierarchy;
};

#endif	  // MESSAGE_ACTIVATE_TRAIL_EDGE_H
