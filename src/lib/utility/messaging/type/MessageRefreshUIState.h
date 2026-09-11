#ifndef MESSAGE_REFRESH_UI_STATE_H
#define MESSAGE_REFRESH_UI_STATE_H

#include "Message.h"

class MessageRefreshUIState: public Message<MessageRefreshUIState>
{
public:
	MessageRefreshUIState(bool isAfterIndexing): isAfterIndexing(isAfterIndexing) {}

	const bool isAfterIndexing = false;
};

#endif	  // MESSAGE_REFRESH_UI_STATE_H
