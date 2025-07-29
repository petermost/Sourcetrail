#ifndef LOG_MANAGER_H
#define LOG_MANAGER_H

#include <memory>

#include "LogManagerImplementation.h"
#include "Logger.h"

class LogManager
{
public:
	static std::shared_ptr<LogManager> getInstance();
	static void destroyInstance();

	void setLoggingEnabled(bool enabled);
	bool getLoggingEnabled() const;

	void addLogger(std::shared_ptr<Logger> logger);
	void removeLogger(std::shared_ptr<Logger> logger);
	void removeLoggersByType(const std::string& type);
	void clearLoggers();
	int getLoggerCount() const;
	Logger* getLoggerByType(const std::string& type);
	Logger* getLogger(std::shared_ptr<Logger> logger);

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
	static std::shared_ptr<LogManager> s_instance;

	LogManager();
	LogManager(const LogManager&);
	LogManager &operator=(const LogManager&);

	LogManagerImplementation m_logManagerImplementation;
	bool m_loggingEnabled = false;
};

#endif	  // LOG_MANAGER_H
