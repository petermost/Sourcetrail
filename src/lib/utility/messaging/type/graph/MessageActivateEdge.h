#ifndef MESSAGE_ACTIVATE_EDGE_H
#define MESSAGE_ACTIVATE_EDGE_H

#include "Edge.h"
#include "NameHierarchy.h"

#include "Message.h"
#include "TabIds.h"

class MessageActivateEdge: public Message<MessageActivateEdge>
{
public:
	MessageActivateEdge(
		Id tokenId,
		Edge::EdgeType type,
		const NameHierarchy& sourceNameHierarchy,
		const NameHierarchy& targetNameHierarchy)
		: tokenId(tokenId)
		, type(type)
		, sourceNameHierarchy(sourceNameHierarchy)
		, targetNameHierarchy(targetNameHierarchy)
	{
		if (!isBundledEdges())
		{
			setKeepContent(true);
		}

		setSchedulerId(TabIds::currentTab());
	}

	static const std::string getStaticType()
	{
		return "MessageActivateEdge";
	}

	bool isBundledEdges() const
	{
		return type == Edge::EDGE_BUNDLED_EDGES;
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
		os << tokenId << " - " << getFullName();
	}

	const Id tokenId;
	const Edge::EdgeType type;
	const NameHierarchy sourceNameHierarchy;
	const NameHierarchy targetNameHierarchy;

	std::vector<Id> bundledEdgesIds;
};

#endif	  // MESSAGE_ACTIVATE_EDGE_H
