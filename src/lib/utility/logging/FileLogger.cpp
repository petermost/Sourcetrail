#include "FileLogger.h"

#include <cstdio>
#include <ctime>
#include <fstream>
#include <sstream>

#include "FileSystem.h"

std::string FileLogger::generateDatedFileName(const std::string &prefix, int offsetDays)
{
	time_t time = std::time(nullptr);
	tm t = *std::localtime(&time);

	// Apply the offset directly to the day field. mktime() will normalizes the struct (e.g., Dec 32nd becomes Jan 1st)
	// and handles Daylight Saving Time transitions correctly:

	t.tm_mday += offsetDays;
	mktime(&t);

	std::stringstream filename;
	filename << prefix << "_" << std::put_time(&t, "%Y-%m-%d_%H-%M-%S");

	return filename.str();
}

FileLogger::FileLogger()
	: Logger("FileLogger")
	, m_logFileName("log")
	, m_logDirectory("user/log/")
{
	updateLogFileName();
}

FilePath FileLogger::getLogFilePath() const
{
	return m_currentLogFilePath;
}

void FileLogger::setLogFilePath(const FilePath& filePath)
{
	m_currentLogFilePath = filePath;
	m_logFileName = "";
}

void FileLogger::setLogDirectory(const FilePath& filePath)
{
	m_logDirectory = filePath;
	FileSystem::createDirectories(m_logDirectory);
}

void FileLogger::setFileName(const std::string& fileName)
{
	if (fileName != m_logFileName)
	{
		m_logFileName = fileName;
		m_currentLogLineCount = 0;
		m_currentLogFileCount = 0;
		updateLogFileName();
	}
}

void FileLogger::logInfo(const LogMessage& message)
{
	logMessage("INFO", message);
}

void FileLogger::logWarning(const LogMessage& message)
{
	logMessage("WARNING", message);
}

void FileLogger::logError(const LogMessage& message)
{
	logMessage("ERROR", message);
}

void FileLogger::setMaxLogLineCount(unsigned int lineCount)
{
	m_maxLogLineCount = lineCount;
}

void FileLogger::setMaxLogFileCount(unsigned int fileCount)
{
	m_maxLogFileCount = fileCount;
}

void FileLogger::deleteLogFiles(const std::string& cutoffDate)
{
	for (const FilePath& file: FileSystem::getFilePathsFromDirectory(m_logDirectory, {".txt"}))
	{
		if (file.fileName() < cutoffDate)
		{
			FileSystem::remove(file);
		}
	}
}

void FileLogger::updateLogFileName()
{
	if (m_logFileName.empty())
	{
		return;
	}

	bool fileChanged = false;

	std::string currentLogFilePath = m_logDirectory.str() + m_logFileName;
	if (m_maxLogFileCount > 0)
	{
		currentLogFilePath += "_";
		if (m_currentLogLineCount >= m_maxLogLineCount)
		{
			m_currentLogLineCount = 0;

			m_currentLogFileCount++;
			if (m_currentLogFileCount >= m_maxLogFileCount)
			{
				m_currentLogFileCount = 0;
			}
			fileChanged = true;
		}
		currentLogFilePath += std::to_string(m_currentLogFileCount);
	}
	currentLogFilePath += ".txt";

	m_currentLogFilePath = FilePath(currentLogFilePath);

	if (fileChanged)
	{
		FileSystem::remove(m_currentLogFilePath);
	}
}

void FileLogger::logMessage(const std::string& type, const LogMessage& message)
{
	std::ofstream fileStream;
	fileStream.open(m_currentLogFilePath.str(), std::ios::app);
	fileStream << message.getTimeString("%H:%M:%S") << " | ";
	fileStream << message.threadId << " | ";

	if (message.filePath.size())
	{
		fileStream << message.getFileName() << ':' << message.line << ' ' << message.functionName
				   << "() | ";
	}

	fileStream << type << ": " << message.message << std::endl;
	fileStream.close();

	m_currentLogLineCount++;
	if (m_maxLogFileCount > 0)
	{
		updateLogFileName();
	}
}
