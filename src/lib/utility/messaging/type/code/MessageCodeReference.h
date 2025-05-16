#ifndef MESSAGE_CODE_REFERENCE_H
#define MESSAGE_CODE_REFERENCE_H

#include "Message.h"
#include "TabIds.h"

class MessageCodeReference: public Message<MessageCodeReference>
{
public:
	enum ReferenceType
	{
		REFERENCE_PREVIOUS,
		REFERENCE_NEXT
	};

	MessageCodeReference(ReferenceType type, bool localReference)
		: type(type), localReference(localReference)
	{
		setSchedulerId(TabIds::currentTab());
	}

	static const std::string getStaticType()
	{
		return "MessageCodeReference";
	}

	void print(std::ostream& os) const override
	{
		if (type == REFERENCE_PREVIOUS)
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

	const ReferenceType type;
	bool localReference;
};

#endif	  // MESSAGE_CODE_REFERENCE_H
