#ifndef MESSAGE_FOCUS_CHANGED_H
#define MESSAGE_FOCUS_CHANGED_H

#include "Message.h"
#include "TabIds.h"

class MessageFocusChanged: public Message<MessageFocusChanged>
{
public:
	enum class ViewType
	{
		GRAPH,
		CODE
	};

	MessageFocusChanged(ViewType type, Id tokenOrLocationId)
		: type(type), tokenOrLocationId(tokenOrLocationId)
	{
		setIsLogged(false);
		setSchedulerId(TabIds::currentTab());
	}

	static const std::string getStaticType()
	{
		return "MessageFocusChanged";
	}

	void print(std::ostream& os) const override
	{
		switch (type)
		{
		case ViewType::GRAPH:
			os << "graph";
			break;
		case ViewType::CODE:
			os << "code";
			break;
		}

		os << " " << tokenOrLocationId;
	}

	bool isFromGraph() const
	{
		return type == ViewType::GRAPH;
	}

	bool isFromCode() const
	{
		return type == ViewType::CODE;
	}

	const ViewType type;
	const Id tokenOrLocationId;
};

#endif	  // MESSAGE_FOCUS_CHANGED_H
