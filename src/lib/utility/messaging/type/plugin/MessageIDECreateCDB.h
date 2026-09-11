#ifndef MESSAGE_IDE_CREATE_CDB_H
#define MESSAGE_IDE_CREATE_CDB_H

#include "Message.h"

class MessageIDECreateCDB: public Message<MessageIDECreateCDB>
{
public:
	MessageIDECreateCDB() = default;

	void print(std::ostream& os) const override
	{
		os << "Create CDB from current solution";
	}
};

#endif	  // MESSAGE_IDE_CREATE_CDB_H
