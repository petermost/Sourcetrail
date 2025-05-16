#include "Catch2.hpp"

#include <thread>

#include "LogManagerImplementation.h"

namespace
{
class TestLogger: public Logger
{
public:
	TestLogger();

	void reset();
	int getMessageCount() const;
	int getWarningCount() const;
	int getErrorCount() const;

	std::string getLastInfo() const;
	std::string getLastWarning() const;
	std::string getLastError() const;

private:
	void logInfo(const LogMessage& message) override;
	void logWarning(const LogMessage& message) override;
	void logError(const LogMessage& message) override;

	int m_logMessageCount = 0;
	int m_logWarningCount = 0;
	int m_logErrorCount = 0;

	std::string m_lastInfo;
	std::string m_lastWarning;
	std::string m_lastError;
};

TestLogger::TestLogger()
	: Logger("TestLogger")
{
}

/*
void TestLogger::reset()
{
	m_logMessageCount = 0;
	m_logWarningCount = 0;
	m_logErrorCount = 0;
}
*/

int TestLogger::getMessageCount() const
{
	return m_logMessageCount;
}

int TestLogger::getWarningCount() const
{
	return m_logWarningCount;
}

int TestLogger::getErrorCount() const
{
	return m_logErrorCount;
}

std::string TestLogger::getLastInfo() const
{
	return m_lastInfo;
}

std::string TestLogger::getLastWarning() const
{
	return m_lastWarning;
}

std::string TestLogger::getLastError() const
{
	return m_lastError;
}

void TestLogger::logInfo(const LogMessage& message)
{
	m_lastInfo = message.message;
	m_logMessageCount++;
}

void TestLogger::logWarning(const LogMessage& message)
{
	m_lastWarning = message.message;
	m_logWarningCount++;
}

void TestLogger::logError(const LogMessage& message)
{
	m_lastError = message.message;
	m_logErrorCount++;
}


void addTestLogger(LogManagerImplementation* logManagerImplementation, const unsigned int loggerCount)
{
	for (unsigned int i = 0; i < loggerCount; i++)
	{
		std::shared_ptr<Logger> logger = std::make_shared<TestLogger>();
		logManagerImplementation->addLogger(logger);
	}
}

void removeTestLoggers(LogManagerImplementation* logManagerImplementation)
{
	std::shared_ptr<Logger> logger = std::make_shared<TestLogger>();
	logManagerImplementation->removeLoggersByType(logger->getType());
}

void addAndRemoveTestLogger(
	LogManagerImplementation* logManagerImplementation, const unsigned int loggerCount)
{
	addTestLogger(logManagerImplementation, loggerCount);
	removeTestLoggers(logManagerImplementation);
}

void logSomeMessages(
	LogManagerImplementation* logManagerImplementation,
	const std::string& message,
	const unsigned int messageCount)
{
	for (unsigned int i = 0; i < messageCount; i++)
	{
		logManagerImplementation->logInfo(message, __FILE__, __FUNCTION__, __LINE__);
		logManagerImplementation->logWarning(message, __FILE__, __FUNCTION__, __LINE__);
		logManagerImplementation->logError(message, __FILE__, __FUNCTION__, __LINE__);
	}
}
}	 // namespace

TEST_CASE("new logger can be added to manager")
{
	LogManagerImplementation logManagerImplementation;

	std::shared_ptr<Logger> logger = std::make_shared<TestLogger>();

	int countBeforeAdd = logManagerImplementation.getLoggerCount();
	logManagerImplementation.addLogger(logger);
	int countAfterAdd = logManagerImplementation.getLoggerCount();
	logManagerImplementation.removeLogger(logger);

	REQUIRE(1 == countAfterAdd - countBeforeAdd);
}

TEST_CASE("logger can be removed from manager")
{
	LogManagerImplementation logManagerImplementation;

	std::shared_ptr<Logger> logger = std::make_shared<TestLogger>();

	int countBeforeAdd = logManagerImplementation.getLoggerCount();
	logManagerImplementation.addLogger(logger);
	logManagerImplementation.removeLogger(logger);
	int countAfterRemove = logManagerImplementation.getLoggerCount();

	REQUIRE(countBeforeAdd == countAfterRemove);
}

TEST_CASE("logger logs message")
{
	LogManagerImplementation logManagerImplementation;

	const std::string log = "test";
	std::shared_ptr<TestLogger> logger = std::make_shared<TestLogger>();

	logManagerImplementation.addLogger(logger);
	logManagerImplementation.logInfo(log, __FILE__, __FUNCTION__, __LINE__);
	const int logCount = logger->getMessageCount();
	const std::string lastLog = logger->getLastInfo();

	REQUIRE(1 == logCount);
	REQUIRE(log == lastLog);
}

TEST_CASE("logger logs warning")
{
	LogManagerImplementation logManagerImplementation;

	const std::string log = "test";
	std::shared_ptr<TestLogger> logger = std::make_shared<TestLogger>();

	logManagerImplementation.addLogger(logger);

	logManagerImplementation.logWarning(log, __FILE__, __FUNCTION__, __LINE__);
	const int logCount = logger->getWarningCount();
	const std::string lastLog = logger->getLastWarning();

	REQUIRE(1 == logCount);
	REQUIRE(log == lastLog);
}

TEST_CASE("logger logs error")
{
	LogManagerImplementation logManagerImplementation;

	std::string log = "test";
	std::shared_ptr<TestLogger> logger = std::make_shared<TestLogger>();

	logManagerImplementation.addLogger(logger);

	logManagerImplementation.logError(log, __FILE__, __FUNCTION__, __LINE__);
	const int logCount = logger->getErrorCount();
	const std::string lastLog = logger->getLastError();

	REQUIRE(1 == logCount);
	REQUIRE(log == lastLog);
}

TEST_CASE("logger logs only logs of defined log level")
{
	LogManagerImplementation logManagerImplementation;

	std::string info = "info";
	std::string warning = "warning";
	std::string error = "error";

	std::shared_ptr<TestLogger> logger = std::make_shared<TestLogger>();

	logger->setLogLevel(Logger::LOG_INFOS | Logger::LOG_ERRORS);

	logManagerImplementation.addLogger(logger);

	logManagerImplementation.logInfo(info, __FILE__, __FUNCTION__, __LINE__);
	logManagerImplementation.logWarning(warning, __FILE__, __FUNCTION__, __LINE__);
	logManagerImplementation.logError(error, __FILE__, __FUNCTION__, __LINE__);

	REQUIRE(1 == logger->getMessageCount());
	REQUIRE(0 == logger->getWarningCount());
	REQUIRE(1 == logger->getErrorCount());

	REQUIRE(info == logger->getLastInfo());
	REQUIRE(error == logger->getLastError());
}

TEST_CASE("new logger can be added to manager threaded")
{
	LogManagerImplementation logManagerImplementation;
	int loggerCount = 100;

	std::thread thread0(addTestLogger, &logManagerImplementation, loggerCount);
	std::thread thread1(addTestLogger, &logManagerImplementation, loggerCount);

	thread0.join();
	thread1.join();

	REQUIRE(loggerCount * 2 == logManagerImplementation.getLoggerCount());
}

TEST_CASE("logger can be removed from manager threaded")
{
	LogManagerImplementation logManagerImplementation;
	unsigned int loggerCount = 100;

	std::thread thread0(addAndRemoveTestLogger, &logManagerImplementation, loggerCount);
	std::thread thread1(addAndRemoveTestLogger, &logManagerImplementation, loggerCount);

	thread0.join();
	thread1.join();

	REQUIRE(0 == logManagerImplementation.getLoggerCount());
}

TEST_CASE("logger logs threaded")
{
	LogManagerImplementation logManagerImplementation;

	std::string log = "foo";
	int messageCount = 100;
	std::shared_ptr<TestLogger> logger = std::make_shared<TestLogger>();
	logManagerImplementation.addLogger(logger);

	std::thread thread0(logSomeMessages, &logManagerImplementation, log, messageCount);
	std::thread thread1(logSomeMessages, &logManagerImplementation, log, messageCount);

	thread0.join();
	thread1.join();

	REQUIRE(logger->getLastError() == log);
	REQUIRE(
		messageCount * 6 ==
		logger->getErrorCount() + logger->getWarningCount() + logger->getMessageCount());
}
