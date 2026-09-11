#ifndef MESSAGE_STATUS_FILTER_CHANGED_H
#define MESSAGE_STATUS_FILTER_CHANGED_H

#include "Message.h"
#include "Status.h"

class MessageStatusFilterChanged: public Message<MessageStatusFilterChanged>
{
public:
	MessageStatusFilterChanged(const StatusFilter filter): statusFilter(filter) {}

	const StatusFilter statusFilter;
};

#endif	  // MESSAGE_STATUS_FILTER_CHANGED_H
