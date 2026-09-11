#ifndef MESSAGE_LOG_FILTER_CHANGED_H
#define MESSAGE_LOG_FILTER_CHANGED_H

#include "Logger.h"
#include "Message.h"

class MessageLogFilterChanged: public Message<MessageLogFilterChanged>
{
public:
	MessageLogFilterChanged(const Logger::LogLevelMask filter): logFilter(filter) {}

	const Logger::LogLevelMask logFilter;
};

#endif	  // MESSAGE_LOG_FILTER_CHANGED_H
