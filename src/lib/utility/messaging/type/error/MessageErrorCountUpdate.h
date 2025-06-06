#ifndef MESSAGE_ERROR_COUNT_UPDATE_H
#define MESSAGE_ERROR_COUNT_UPDATE_H

#include "Message.h"

#include "ErrorCountInfo.h"

class MessageErrorCountUpdate: public Message<MessageErrorCountUpdate>
{
public:
	static const std::string getStaticType()
	{
		return "MessageErrorCountUpdate";
	}

	MessageErrorCountUpdate(const ErrorCountInfo& errorCount, const std::vector<ErrorInfo>& newErrors)
		: errorCount(errorCount), newErrors(newErrors)
	{
		setSendAsTask(false);
	}

	void print(std::ostream& os) const override
	{
		os << errorCount.total << '/' << errorCount.fatal << " - " << newErrors.size()
		   << " new errors";
	}

	const ErrorCountInfo errorCount;
	std::vector<ErrorInfo> newErrors;
};

#endif	  // MESSAGE_ERROR_COUNT_UPDATE_H
