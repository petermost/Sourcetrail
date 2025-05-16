#ifndef MESSAGE_ZOOM_H
#define MESSAGE_ZOOM_H

#include "Message.h"

class MessageZoom: public Message<MessageZoom>
{
public:
	MessageZoom(bool zoomIn): zoomIn(zoomIn) {}

	static const std::string getStaticType()
	{
		return "MessageZoom";
	}

	void print(std::ostream& os) const override
	{
		if (zoomIn)
		{
			os << "in";
		}
		else
		{
			os << "out";
		}
	}

	const bool zoomIn;
};

#endif	  // MESSAGE_ZOOM_H
