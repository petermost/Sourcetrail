#include "LogManagerImplementation.h"

#include <algorithm>

using namespace aidkit;

static tm getTime()
{
	time_t time = std::time(nullptr);
	return *std::localtime(&time);
}

void LogManagerImplementation::addLogger(std::shared_ptr<Logger> logger)
{
	m_loggers.access()->push_back(logger);
}

void LogManagerImplementation::removeLogger(std::shared_ptr<Logger> logger)
{
	concurrent::access([&](auto &loggers)
	{
		std::erase(loggers, logger);
	}, m_loggers);
}

void LogManagerImplementation::removeLoggersByType(const std::string &type)
{
	concurrent::access([&](auto &loggers)
	{
		std::erase_if(loggers, [&](std::shared_ptr<Logger> &logger)
		{
			return logger->getType() == type;
		});
	}, m_loggers);
}

Logger *LogManagerImplementation::getLogger(std::shared_ptr<Logger> logger)
{
	return concurrent::access([&](auto &loggers)
	{
		auto it = std::find(loggers.begin(), loggers.end(), logger);
		if (it != loggers.end())
		{
			return it->get();
		}
		return static_cast<Logger *>(nullptr);
	}, m_loggers);
}

Logger *LogManagerImplementation::getLoggerByType(const std::string &type)
{
	return concurrent::access([&](auto &loggers)
	{
		auto it = std::find_if(loggers.begin(), loggers.end(), [&](std::shared_ptr<Logger> &logger)
		{
			return logger->getType() == type;
		});
		if (it != loggers.end())
		{
			return it->get();
		}
		return static_cast<Logger *>(nullptr);
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

void LogManagerImplementation::logInfo(const std::string &message, const std::string &file, const std::string &function, const unsigned int line)
{
	concurrent::access([&](auto &loggers)
	{
		for (std::shared_ptr<Logger> &logger : loggers)
		{
			logger->onInfo(LogMessage(message, file, function, line, getTime(), std::this_thread::get_id()));
		}
	}, m_loggers);
}

void LogManagerImplementation::logWarning(const std::string &message, const std::string &file, const std::string &function, const unsigned int line)
{
	concurrent::access([&](auto &loggers)
	{
		for (std::shared_ptr<Logger> &logger : loggers)
		{
			logger->onWarning(LogMessage(message, file, function, line, getTime(), std::this_thread::get_id()));
		}
	}, m_loggers);
}

void LogManagerImplementation::logError(const std::string &message, const std::string &file, const std::string &function, const unsigned int line)
{
	concurrent::access([&](auto &loggers)
	{
		for (std::shared_ptr<Logger> &logger : loggers)
		{
			logger->onError(LogMessage(message, file, function, line, getTime(), std::this_thread::get_id()));
		}
	}, m_loggers);
}
