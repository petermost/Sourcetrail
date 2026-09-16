#ifndef MESSAGE_REFRESH_H
#define MESSAGE_REFRESH_H

#include "Message.h"
#include "RefreshInfo.h"

class MessageRefresh: public Message<MessageRefresh>
{
public:
	MessageRefresh(RefreshMode refreshMode = RefreshMode::UPDATED_FILES)
		: refreshMode(refreshMode)
	{
	}

	void print(std::ostream& os) const override
	{
		if (refreshMode == RefreshMode::ALL_FILES)
		{
			os << "all";
		}
	}
	const RefreshMode refreshMode;
};

#endif	  // MESSAGE_REFRESH_H
