#ifndef LOG_MANAGER_IMPLEMENTATION_H
#define LOG_MANAGER_IMPLEMENTATION_H

#include "Logger.h"

#include <aidkit/thread_shared.hpp>

#include <memory>
#include <vector>

class LogManagerImplementation
{
public:
	void addLogger(std::shared_ptr<Logger> logger);
	void removeLogger(std::shared_ptr<Logger> logger);
	void removeLoggersByType(const std::string& type);
	void clearLoggers();
	int getLoggerCount() const;
	Logger* getLogger(std::shared_ptr<Logger> logger);
	Logger* getLoggerByType(const std::string& type);

	void logInfo(
		const std::string& message,
		const std::string& file,
		const std::string& function,
		const unsigned int line);
	void logWarning(
		const std::string& message,
		const std::string& file,
		const std::string& function,
		const unsigned int line);
	void logError(
		const std::string& message,
		const std::string& file,
		const std::string& function,
		const unsigned int line);

private:
	aidkit::thread_shared<std::vector<std::shared_ptr<Logger>>> m_loggers;
};

#endif	  // LOG_MANAGER_IMPLEMENTATION_H
