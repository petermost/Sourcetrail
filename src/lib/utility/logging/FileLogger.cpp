#include "FileLogger.h"

#include <cstdio>
#include <ctime>
#include <fstream>
#include <sstream>

#include "FileSystem.h"
#include "utilityString.h"

std::string FileLogger::generateDatedFileName(
	const std::string& prefix, const std::string& suffix, int offsetDays)
{
	time_t time;
	std::time(&time);

	tm t = *std::localtime(&time);

	if (offsetDays != 0)
	{
		time = mktime(&t) + offsetDays * 24 * 60 * 60;
		t = *std::localtime(&time);
	}

	std::stringstream filename;
	if (!prefix.empty())
	{
		filename << prefix << "_";
	}

	filename << t.tm_year + 1900 << "-";
	filename << (t.tm_mon < 9 ? "0" : "") << t.tm_mon + 1 << "-";
	filename << (t.tm_mday < 10 ? "0" : "") << t.tm_mday << "_";
	filename << (t.tm_hour < 10 ? "0" : "") << t.tm_hour << "-";
	filename << (t.tm_min < 10 ? "0" : "") << t.tm_min << "-";
	filename << (t.tm_sec < 10 ? "0" : "") << t.tm_sec;

	if (!suffix.empty())
	{
		filename << "_" << suffix;
	}

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

	fileStream << type << ": " << utility::encodeToUtf8(message.message) << std::endl;
	fileStream.close();

	m_currentLogLineCount++;
	if (m_maxLogFileCount > 0)
	{
		updateLogFileName();
	}
}
