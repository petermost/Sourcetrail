#ifndef MESSAGE_REFRESH_H
#define MESSAGE_REFRESH_H

#include "Message.h"

class MessageRefresh: public Message<MessageRefresh>
{
public:
	MessageRefresh() = default;

	MessageRefresh& refreshAll()
	{
		all = true;
		return *this;
	}

	void print(std::ostream& os) const override
	{
		if (all)
		{
			os << "all";
		}
	}

	bool all = false;
};

#endif	  // MESSAGE_REFRESH_H
