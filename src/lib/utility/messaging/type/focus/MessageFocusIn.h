#ifndef MESSAGE_FOCUS_IN_H
#define MESSAGE_FOCUS_IN_H

#include <vector>

#include "Message.h"
#include "TabIds.h"
#include "TooltipOrigin.h"
#include "types.h"

class MessageFocusIn: public Message<MessageFocusIn>
{
public:
	MessageFocusIn(const std::vector<Id>& tokenIds, TooltipOrigin origin = TOOLTIP_ORIGIN_NONE)
		: tokenIds(tokenIds), origin(origin)
	{
		setIsLogged(false);
		setSchedulerId(TabIds::currentTab());
	}

	static const std::string getStaticType()
	{
		return "MessageFocusIn";
	}

	void print(std::ostream& os) const override
	{
		for (const Id& id: tokenIds)
		{
			os << id << " ";
		}
	}

	const std::vector<Id> tokenIds;
	const TooltipOrigin origin;
};

#endif	  // MESSAGE_FOCUS_IN_H
