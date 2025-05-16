#include "NetworkProtocolHelper.h"

#include <sstream>
#include <string>

#include <boost/algorithm/string.hpp>

#include "logging.h"
#include "utilityString.h"

std::string NetworkProtocolHelper::s_divider = ">>";
std::string NetworkProtocolHelper::s_setActiveTokenPrefix = "setActiveToken";
std::string NetworkProtocolHelper::s_moveCursorPrefix = "moveCursor";
std::string NetworkProtocolHelper::s_endOfMessageToken = "<EOM>";
std::string NetworkProtocolHelper::s_createProjectPrefix = "createProject";
std::string NetworkProtocolHelper::s_createCDBProjectPrefix = "createCDBProject";
std::string NetworkProtocolHelper::s_createCDBPrefix = "createCDB";
std::string NetworkProtocolHelper::s_pingPrefix = "ping";

NetworkProtocolHelper::MESSAGE_TYPE NetworkProtocolHelper::getMessageType(const std::string& message)
{
	std::vector<std::string> subMessages = divideMessage(message);

	if (!subMessages.empty())
	{
		if (subMessages[0] == s_setActiveTokenPrefix)
		{
			return MESSAGE_TYPE::SET_ACTIVE_TOKEN;
		}
		else if (subMessages[0] == s_createProjectPrefix)
		{
			return MESSAGE_TYPE::CREATE_PROJECT;
		}
		else if (subMessages[0] == s_createCDBProjectPrefix)
		{
			return MESSAGE_TYPE::CREATE_CDB_MESSAGE;
		}
		else if (subMessages[0] == s_pingPrefix)
		{
			return MESSAGE_TYPE::PING;
		}
		else
		{
			return MESSAGE_TYPE::UNKNOWN;
		}
	}

	return MESSAGE_TYPE::UNKNOWN;
}

NetworkProtocolHelper::SetActiveTokenMessage NetworkProtocolHelper::parseSetActiveTokenMessage(
	const std::string& message)
{
	std::vector<std::string> subMessages = divideMessage(message);

	SetActiveTokenMessage networkMessage;

	if (!subMessages.empty())
	{
		if (subMessages[0] == s_setActiveTokenPrefix)
		{
			if (subMessages.size() != 5)
			{
				LOG_ERROR("Failed to parse setActiveToken message, invalid token count");
			}
			else
			{
				const std::string filePath = subMessages[1];
				const std::string row = subMessages[2];
				const std::string column = subMessages[3];

				if (!filePath.empty() && !row.empty() && !column.empty() && isDigits(row) &&
					isDigits(column))
				{
					networkMessage.filePath = FilePath(filePath);
					networkMessage.row = std::stoi(row);
					networkMessage.column = std::stoi(column);
					networkMessage.valid = true;
				}
			}
		}
		else
		{
			LOG_ERROR(
				"Failed to parse message, invalid type token: " + subMessages[0] + ". Expected " +
				s_setActiveTokenPrefix);
		}
	}

	return networkMessage;
}

NetworkProtocolHelper::CreateProjectMessage NetworkProtocolHelper::parseCreateProjectMessage(
	const std::string& message)
{
	std::vector<std::string> subMessages = divideMessage(message);

	NetworkProtocolHelper::CreateProjectMessage networkMessage;

	if (!subMessages.empty())
	{
		if (subMessages[0] == s_createProjectPrefix)
		{
			if (subMessages.size() != 4)
			{
				LOG_ERROR("Failed to parse createProject message, invalid token count");
			}
		}
		else
		{
			LOG_ERROR(
				"Failed to parse message, invalid type token: " + subMessages[0] + ". Expected " +
				s_createProjectPrefix);
		}
	}

	return networkMessage;
}

NetworkProtocolHelper::CreateCDBProjectMessage NetworkProtocolHelper::parseCreateCDBProjectMessage(
	const std::string& message)
{
	std::vector<std::string> subMessages = divideMessage(message);

	NetworkProtocolHelper::CreateCDBProjectMessage networkMessage;

	if (!subMessages.empty())
	{
		if (subMessages[0] == s_createCDBProjectPrefix)
		{
			if (subMessages.size() < 4)
			{
				LOG_ERROR("Failed to parse createCDBProject message, too few tokens");
			}
			else
			{
				const size_t subMessageCount = subMessages.size();

				const std::string cdbPath = subMessages[1];
				if (!cdbPath.empty())
				{
					networkMessage.cdbFileLocation = FilePath(cdbPath);
				}
				else
				{
					LOG_WARNING("CDB file path is not set.");
				}

				const std::string ideId = subMessages[subMessageCount - 2];
				if (!ideId.empty())
				{
					std::string nonConstId = ideId;
					boost::algorithm::to_lower(nonConstId);

					networkMessage.ideId = nonConstId;
				}
				else
				{
					LOG_WARNING("Failed to parse ide ID string. Is " + ideId);
				}

				if (!networkMessage.cdbFileLocation.empty() && !networkMessage.ideId.empty())
				{
					networkMessage.valid = true;
				}
			}
		}
		else
		{
			LOG_ERROR(
				"Failed to parse message, invalid type token: " + subMessages[0] + ". Expected " +
				s_createCDBProjectPrefix);
		}
	}

	return networkMessage;
}

NetworkProtocolHelper::PingMessage NetworkProtocolHelper::parsePingMessage(const std::string& message)
{
	std::vector<std::string> subMessages = divideMessage(message);

	NetworkProtocolHelper::PingMessage pingMessage;

	if (!subMessages.empty())
	{
		if (subMessages[0] == s_pingPrefix)
		{
			if (subMessages.size() < 2)
			{
				LOG_ERROR_STREAM(<< "Failed to parse PingMessage message, too few tokens");
			}
			else
			{
				std::string ideId = subMessages[1];

				if (!ideId.empty())
				{
					std::string nonConstId = ideId;
					boost::algorithm::to_lower(nonConstId);

					pingMessage.ideId = ideId;

					pingMessage.valid = true;
				}
				else
				{
					LOG_WARNING("Failed to parse ide ID string: " + ideId);
				}
			}
		}
		else
		{
			LOG_ERROR(
				"Failed to parse message, invalid type token: " + subMessages[0] + ". Expected " +
				s_pingPrefix);
		}
	}

	return pingMessage;
}

std::string NetworkProtocolHelper::buildSetIDECursorMessage(
	const FilePath& fileLocation, const unsigned int row, const unsigned int column)
{
	std::stringstream messageStream;

	messageStream << s_moveCursorPrefix;
	messageStream << s_divider;
	messageStream << fileLocation.str();
	messageStream << s_divider;
	messageStream << row;
	messageStream << s_divider;
	messageStream << column;
	messageStream << s_endOfMessageToken;

	return messageStream.str();
}

std::string NetworkProtocolHelper::buildCreateCDBMessage()
{
	std::stringstream messageStream;

	messageStream << s_createCDBPrefix;
	messageStream << s_endOfMessageToken;

	return messageStream.str();
}

std::string NetworkProtocolHelper::buildPingMessage()
{
	std::stringstream messageStream;

	messageStream << s_pingPrefix;
	messageStream << s_divider;
	messageStream << "sourcetrail";
	messageStream << s_endOfMessageToken;

	return messageStream.str();
}

std::vector<std::string> NetworkProtocolHelper::divideMessage(const std::string& message)
{
	std::vector<std::string> result;

	std::string msg = message;
	size_t pos = msg.find(s_divider);

	while (pos != std::string::npos)
	{
		std::string subMessage = msg.substr(0, pos);
		result.push_back(subMessage);
		msg = msg.substr(pos + s_divider.size());
		pos = msg.find(s_divider);
	}

	if (msg.size() > 0)
	{
		pos = msg.find(s_endOfMessageToken);
		if (pos != std::string::npos)
		{
			std::string subMessage = msg.substr(0, pos);
			result.push_back(subMessage);
			msg = msg.substr(pos);
			result.push_back(msg);
		}
	}

	return result;
}

bool NetworkProtocolHelper::isDigits(const std::string& text)
{
	return (text.find_first_not_of("0123456789") == std::string::npos);
}
