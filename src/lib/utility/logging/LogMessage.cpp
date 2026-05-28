#include "LogMessage.h"

LogMessage::LogMessage(const std::string &message, const std::string &filePath, const std::string &functionName, const unsigned int line,
	const tm &time, const std::thread::id &threadId)
	: message(message)
	, filePath(filePath)
	, functionName(functionName)
	, line(line)
	, time(time)
	, threadId(threadId)
{
}

std::string LogMessage::getTimeString(const std::string &format) const
{
	char timeString[50];
	strftime(timeString, 50, format.c_str(), &time);
	return std::string(timeString);
}

std::string LogMessage::getFileName() const
{
	return filePath.substr(filePath.find_last_of("/\\") + 1);
}
