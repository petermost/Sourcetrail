#ifndef STATUS_H
#define STATUS_H

#include <string>

enum StatusType
{
	STATUS_INFO = 1,
	STATUS_ERROR = 2,
};

typedef int StatusFilter;

struct Status
{
	Status(std::string message, bool isError = false)
		: message(message), type(isError ? StatusType::STATUS_ERROR : StatusType::STATUS_INFO)
	{
	}

	std::string message;
	StatusType type;
};

#endif	  // STATUS_H
