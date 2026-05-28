#ifndef LOG_MESSAGE_H
#define LOG_MESSAGE_H

#include <ctime>
#include <string>
#include <thread>

struct LogMessage
{
public:
	LogMessage(
		const std::string& message,
		const std::string& filePath,
		const std::string& functionName,
		const unsigned int line,
		const std::tm& time,
		const std::thread::id& threadId);

	std::string getTimeString(const std::string& format) const;

	std::string getFileName() const;

	const std::string message;
	const std::string filePath;
	const std::string functionName;
	const unsigned int line;
	const std::tm time;
	const std::thread::id threadId;
};

#endif	  // LOG_MESSAGE_H
