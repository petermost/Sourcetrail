#ifndef MESSAGE_INDEXING_FINISHED_H
#define MESSAGE_INDEXING_FINISHED_H

#include "Message.h"

class MessageIndexingFinished: public Message<MessageIndexingFinished>
{
public:
	MessageIndexingFinished() = default;
};

#endif	  // MESSAGE_INDEXING_FINISHED_H
