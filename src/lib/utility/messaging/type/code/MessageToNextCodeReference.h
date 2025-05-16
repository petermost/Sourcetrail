#ifndef MESSAGE_TO_NEXT_CODE_REFERENCE_H
#define MESSAGE_TO_NEXT_CODE_REFERENCE_H

#include "Message.h"
#include "TabIds.h"

class MessageToNextCodeReference: public Message<MessageToNextCodeReference>
{
public:
	MessageToNextCodeReference(const FilePath& filePath, size_t lineNumber, size_t columnNumber, bool next)
		: filePath(filePath), lineNumber(lineNumber), columnNumber(columnNumber), next(next)
	{
		setSchedulerId(TabIds::currentTab());
	}

	static const std::string getStaticType()
	{
		return "MessageToNextCodeReference";
	}

	void print(std::ostream& os) const override
	{
		os << filePath.str() << ' ' << lineNumber << ':' << columnNumber << ' ';

		if (next)
		{
			os << "next";
		}
		else
		{
			os << "previous";
		}
	}

	const FilePath filePath;
	const size_t lineNumber;
	const size_t columnNumber;
	const bool next;
};

#endif	  // MESSAGE_TO_NEXT_CODE_REFERENCE_H
