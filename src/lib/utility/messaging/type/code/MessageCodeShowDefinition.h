#ifndef MESSAGE_CODE_SHOW_DEFINITION_H
#define MESSAGE_CODE_SHOW_DEFINITION_H

#include "Message.h"
#include "TabIds.h"
#include "types.h"

class MessageCodeShowDefinition: public Message<MessageCodeShowDefinition>
{
public:
	static const std::string getStaticType()
	{
		return "MessageCodeShowDefinition";
	}

	MessageCodeShowDefinition(Id nodeId, bool inIDE = false): nodeId(nodeId), inIDE(inIDE)
	{
		setSchedulerId(TabIds::currentTab());
	}

	void print(std::ostream& os) const override
	{
		os << nodeId;
	}

	const Id nodeId;
	const bool inIDE;
};

#endif	  // MESSAGE_CODE_SHOW_DEFINITION_H
