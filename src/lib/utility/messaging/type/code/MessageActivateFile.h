#ifndef MESSAGE_ACTIVATE_FILE_H
#define MESSAGE_ACTIVATE_FILE_H

#include "FilePath.h"
#include "Message.h"
#include "TabIds.h"

class MessageActivateFile: public Message<MessageActivateFile>
{
public:
	MessageActivateFile(const FilePath& filePath, unsigned int line = 0)
		: filePath(filePath), line(line)
	{
		setSchedulerId(TabIds::currentTab());
	}

	static const std::string getStaticType()
	{
		return "MessageActivateFile";
	}

	void print(std::ostream& os) const override
	{
		os << filePath.str();
	}

	const FilePath filePath;
	unsigned int line;
};

#endif	  // MESSAGE_ACTIVATE_FILE_H
