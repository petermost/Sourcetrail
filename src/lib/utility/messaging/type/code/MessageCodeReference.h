#ifndef MESSAGE_CODE_REFERENCE_H
#define MESSAGE_CODE_REFERENCE_H

#include "Message.h"
#include "TabIds.h"

class MessageCodeReference: public Message<MessageCodeReference>
{
public:
	enum class Type
	{
		PREVIOUS,
		NEXT
	};

	MessageCodeReference(Type type, bool localReference)
		: type(type), localReference(localReference)
	{
		setSchedulerId(TabIds::currentTab());
	}

	void print(std::ostream& os) const override
	{
		if (type == Type::PREVIOUS)
		{
			os << "previous";
		}
		else
		{
			os << "next";
		}

		if (localReference)
		{
			os << " local";
		}
	}

	const Type type;
	const bool localReference;
};

#endif	  // MESSAGE_CODE_REFERENCE_H
