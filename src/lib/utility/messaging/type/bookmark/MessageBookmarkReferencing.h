#ifndef MESSAGE_BOOKMARK_REFERENCING_H
#define MESSAGE_BOOKMARK_REFERENCING_H

#include "Message.h"

class MessageBookmarkReferencing: public Message<MessageBookmarkReferencing>
{
public:
	MessageBookmarkReferencing(Id nodeId = 0): nodeId(nodeId) {}

	static const std::string getStaticType()
	{
		return "MessageBookmarkReferencing";
	}

	const Id nodeId;
};

#endif	  // MESSAGE_BOOKMARK_REFERENCING_H