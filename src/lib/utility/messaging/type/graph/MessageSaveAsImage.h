#ifndef MESSAGE_SAVE_AS_IMAGE_H
#define MESSAGE_SAVE_AS_IMAGE_H

#include "Message.h"


class MessageSaveAsImage: public Message<MessageSaveAsImage>
{
public:
	MessageSaveAsImage(const QString &path) : path(path) {}

	const QString path;
};

#endif /* MESSAGE_SAVE_AS_IMAGE_H */
