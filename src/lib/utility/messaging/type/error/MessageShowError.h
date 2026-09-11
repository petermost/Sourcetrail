#ifndef MESSAGE_SHOW_ERROR_H
#define MESSAGE_SHOW_ERROR_H

#include "Message.h"
#include "TabIds.h"

class MessageShowError: public Message<MessageShowError>
{
public:
	MessageShowError(Id errorId): errorId(errorId)
	{
		setSchedulerId(TabIds::currentTab());
	}

	const Id errorId;
};

#endif	  // MESSAGE_SHOW_ERROR_H
