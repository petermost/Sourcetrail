#ifndef MESSAGE_SCROLL_GRAPH_H
#define MESSAGE_SCROLL_GRAPH_H

#include "Message.h"
#include "TabIds.h"

class MessageScrollGraph: public Message<MessageScrollGraph>
{
public:
	MessageScrollGraph(int xValue, int yValue): xValue(xValue), yValue(yValue)
	{
		setIsLogged(false);
		setSchedulerId(TabIds::currentTab());
	}

	int xValue;
	int yValue;
};

#endif	  // MESSAGE_SCROLL_GRAPH_H
