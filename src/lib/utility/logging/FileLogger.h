#ifndef FILE_LOGGER_H
#define FILE_LOGGER_H

#include <string>

#include "FilePath.h"
#include "LogMessage.h"
#include "Logger.h"

class FileLogger: public Logger
{
public:
	static std::string generateDatedFileName(
		const std::string& prefix = "", const std::string& suffix = "", int offsetDays = 0);

	FileLogger();

	FilePath getLogFilePath() const;
	void setLogFilePath(const FilePath& filePath);

	void setLogDirectory(const FilePath& filePath);
	void setFileName(const std::string& fileName);
	void setMaxLogLineCount(unsigned int logCount);

	// setting the max log file count to 0 will disable ringlogging
	void setMaxLogFileCount(unsigned int amount);

	void deleteLogFiles(const std::string& cutoffDate);

private:
	void logInfo(const LogMessage& message) override;
	void logWarning(const LogMessage& message) override;
	void logError(const LogMessage& message) override;

	void logMessage(const std::string& type, const LogMessage& message);
	void updateLogFileName();

	std::string m_logFileName;
	FilePath m_logDirectory;
	FilePath m_currentLogFilePath;

	unsigned int m_maxLogLineCount = 0;
	unsigned int m_maxLogFileCount = 0;
	unsigned int m_currentLogLineCount = 0;
	unsigned int m_currentLogFileCount = 0;
};

#endif	  // FILE_LOGGER_H
