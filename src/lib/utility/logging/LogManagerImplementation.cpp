#include "LogManagerImplementation.h"

#include <algorithm>

static tm getTime()
{
	time_t time;
	std::time(&time);

	// Return copy because localtime returns a pointer to a statically allocated object:

	return *std::localtime(&time);
}

void LogManagerImplementation::addLogger(std::shared_ptr<Logger> logger)
{
	m_loggers.access()->push_back(logger);
}

void LogManagerImplementation::removeLogger(std::shared_ptr<Logger> logger)
{
	aidkit::access([&logger](auto &loggers)
	{
		auto it = std::find(loggers.begin(), loggers.end(), logger);
		if (it != loggers.end())
		{
			loggers.erase(it);
		}
	}, m_loggers);
}

void LogManagerImplementation::removeLoggersByType(const std::string& type)
{
	aidkit::access([&type](auto &loggers)
	{
		for (unsigned int i = 0; i < loggers.size(); i++)
		{
			if (loggers[i]->getType() == type)
			{
				loggers.erase(loggers.begin() + i);
				i--;
			}
		}
	}, m_loggers);
}

Logger* LogManagerImplementation::getLogger(std::shared_ptr<Logger> logger)
{
	return aidkit::access([&logger](auto &loggers)
	{
		auto it = std::find(loggers.begin(), loggers.end(), logger);
		if (it != loggers.end())
		{
			return (*it).get();
		}
		return (Logger *)nullptr;
	}, m_loggers);
}

Logger* LogManagerImplementation::getLoggerByType(const std::string& type)
{
	return aidkit::access([&type](auto &loggers)
	{
		for (unsigned int i = 0; i < loggers.size(); i++)
		{
			if (loggers[i]->getType() == type)
			{
				return loggers[i].get();
			}
		}
		return (Logger *)nullptr;
	}, m_loggers);
}

void LogManagerImplementation::clearLoggers()
{
	m_loggers.access()->clear();
}

int LogManagerImplementation::getLoggerCount() const
{
	return static_cast<int>(m_loggers.access()->size());
}

void LogManagerImplementation::logInfo(
	const std::string& message,
	const std::string& file,
	const std::string& function,
	const unsigned int line)
{
	aidkit::access([&](auto &loggers)
	{
		for (unsigned int i = 0; i < loggers.size(); i++)
		{
			loggers[i]->onInfo(LogMessage(message, file, function, line, getTime(), std::this_thread::get_id()));
		}
	}, m_loggers);
}

void LogManagerImplementation::logWarning(
	const std::string& message,
	const std::string& file,
	const std::string& function,
	const unsigned int line)
{
	aidkit::access([&](auto &loggers)
	{
		for (unsigned int i = 0; i < loggers.size(); i++)
		{
			loggers[i]->onWarning(LogMessage(message, file, function, line, getTime(), std::this_thread::get_id()));
		}
	}, m_loggers);
}

void LogManagerImplementation::logError(
	const std::string& message,
	const std::string& file,
	const std::string& function,
	const unsigned int line)
{
	aidkit::access([&](auto &loggers)
	{
		for (unsigned int i = 0; i < loggers.size(); i++)
		{
			loggers[i]->onError(LogMessage(message, file, function, line, getTime(), std::this_thread::get_id()));
		}
	}, m_loggers);
}

