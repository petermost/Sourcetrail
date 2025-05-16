#ifndef MESSAGE_MOVE_IDE_CURSOR_H
#define MESSAGE_MOVE_IDE_CURSOR_H

#include "FilePath.h"
#include "Message.h"

class MessageMoveIDECursor: public Message<MessageMoveIDECursor>
{
public:
	MessageMoveIDECursor(const FilePath& filePath, const unsigned int row, const unsigned int column)
		: filePath(filePath), row(row), column(column)
	{
	}

	static const std::string getStaticType()
	{
		return "MessageMoveIDECursor";
	}

	void print(std::ostream& os) const override
	{
		os << filePath.str() << ":" << row << ":" << column;
	}

	const FilePath filePath;
	const unsigned int row;
	const unsigned int column;
};

#endif	  // MESSAGE_MOVE_IDE_CURSOR_H