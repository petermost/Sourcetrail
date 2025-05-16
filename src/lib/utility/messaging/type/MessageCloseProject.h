#ifndef MESSAGE_CLOSE_PROJECT_H
#define MESSAGE_CLOSE_PROJECT_H

#include "RefreshInfo.h"

#include "FilePath.h"
#include "Message.h"

class MessageCloseProject: public Message<MessageCloseProject>
{
public:
	MessageCloseProject() = default;

	static const std::string getStaticType()
	{
		return "MessageCloseProject";
	}

	void print(std::ostream&  /*os*/) const override {}
};

#endif	  // MESSAGE_CLOSE_PROJECT_H
