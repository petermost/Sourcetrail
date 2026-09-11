#ifndef MESSAGE_FOCUS_OUT_H
#define MESSAGE_FOCUS_OUT_H

#include <vector>

#include "Message.h"
#include "TabIds.h"
#include "types.h"

class MessageFocusOut: public Message<MessageFocusOut>
{
public:
	MessageFocusOut(const std::vector<Id>& tokenIds): tokenIds(tokenIds)
	{
		setIsLogged(false);
		setSchedulerId(TabIds::currentTab());
	}

	void print(std::ostream& os) const override
	{
		for (const Id& id: tokenIds)
		{
			os << id << " ";
		}
	}

	const std::vector<Id> tokenIds;
};

#endif	  // MESSAGE_FOCUS_OUT_H
