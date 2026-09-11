#ifndef MESSAGE_FIND_H
#define MESSAGE_FIND_H

#include "Message.h"
#include "TabIds.h"

class MessageFind: public Message<MessageFind>
{
public:
	MessageFind(bool fulltext = false): findFulltext(fulltext)
	{
		setSchedulerId(TabIds::currentTab());
	}

	const bool findFulltext;
};

#endif	  // MESSAGE_FIND_H
