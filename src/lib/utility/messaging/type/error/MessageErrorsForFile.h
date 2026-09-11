#ifndef MESSAGE_ERRORS_FOR_FILE_H
#define MESSAGE_ERRORS_FOR_FILE_H

#include "Message.h"

class MessageErrorsForFile: public Message<MessageErrorsForFile>
{
public:
	MessageErrorsForFile(const FilePath& file): file(file) {}

	const FilePath file;
};

#endif	  // MESSAGE_ERRORS_FOR_FILE_H
