#ifndef MESSAGE_BOOKMARK_REFERENCES_H
#define MESSAGE_BOOKMARK_REFERENCES_H

#include "Message.h"

class MessageBookmarkReferences: public Message<MessageBookmarkReferences>
{
public:
	MessageBookmarkReferences(Id nodeId = 0): nodeId(nodeId) {}

	static const std::string getStaticType()
	{
		return "MessageBookmarkReferences";
	}

	const Id nodeId;
};

#endif	  // MESSAGE_BOOKMARK_REFERENCES_H