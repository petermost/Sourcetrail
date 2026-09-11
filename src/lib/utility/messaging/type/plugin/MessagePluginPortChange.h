#ifndef MESSAGE_PLUGIN_PORT_CHANGE_H
#define MESSAGE_PLUGIN_PORT_CHANGE_H

#include "Message.h"

class MessagePluginPortChange: public Message<MessagePluginPortChange>
{
public:
	MessagePluginPortChange() = default;
};

#endif	  // MESSAGE_PLUGIN_PORT_CHANGE_H
