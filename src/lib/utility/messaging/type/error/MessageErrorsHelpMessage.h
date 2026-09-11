#ifndef MESSAGE_ERRORS_HELP_MESSAGE_H
#define MESSAGE_ERRORS_HELP_MESSAGE_H

#include "Message.h"

class MessageErrorsHelpMessage: public Message<MessageErrorsHelpMessage>
{
public:
	MessageErrorsHelpMessage(bool force = false): force(force) {}

	const bool force;
};

#endif	  // MESSAGE_ERRORS_HELP_MESSAGE_H
