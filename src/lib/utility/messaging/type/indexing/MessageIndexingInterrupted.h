#ifndef MESSAGE_INDEXING_INTERRUPTED_H
#define MESSAGE_INDEXING_INTERRUPTED_H

#include "Message.h"

class MessageIndexingInterrupted: public Message<MessageIndexingInterrupted>
{
public:
	MessageIndexingInterrupted()
	{
		setSendAsTask(false);
	}
};

#endif	  // MESSAGE_INDEXING_INTERRUPTED_H
