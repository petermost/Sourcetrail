#ifndef NETWORK_PROTOCOL_HELPER_H
#define NETWORK_PROTOCOL_HELPER_H

#include <string>
#include <vector>

#include "FilePath.h"

class NetworkProtocolHelper
{
public:
	struct SetActiveTokenMessage
	{
	public:
		SetActiveTokenMessage(): filePath("") {}

		FilePath filePath;
		unsigned int row = 0;
		unsigned int column = 0;
		bool valid = false;
	};

	struct CreateProjectMessage
	{
	};

	struct CreateCDBProjectMessage
	{
	public:
		CreateCDBProjectMessage(): cdbFileLocation("")  {}

		FilePath cdbFileLocation;
		std::string ideId;
		bool valid = false;
	};

	struct PingMessage
	{
	public:
		PingMessage() = default;

		std::string ideId;
		bool valid = false;
	};

	enum MESSAGE_TYPE
	{
		UNKNOWN = 0,
		SET_ACTIVE_TOKEN,
		CREATE_PROJECT,
		CREATE_CDB_MESSAGE,
		PING
	};

	static MESSAGE_TYPE getMessageType(const std::string& message);

	static SetActiveTokenMessage parseSetActiveTokenMessage(const std::string& message);
	static CreateProjectMessage parseCreateProjectMessage(const std::string& message);
	static CreateCDBProjectMessage parseCreateCDBProjectMessage(const std::string& message);
	static PingMessage parsePingMessage(const std::string& message);

	static std::string buildSetIDECursorMessage(
		const FilePath& fileLocation, const unsigned int row, const unsigned int column);
	static std::string buildCreateCDBMessage();
	static std::string buildPingMessage();

private:
	static std::vector<std::string> divideMessage(const std::string& message);
	static bool isDigits(const std::string& text);

	static std::string s_divider;
	static std::string s_setActiveTokenPrefix;
	static std::string s_moveCursorPrefix;
	static std::string s_endOfMessageToken;
	static std::string s_createProjectPrefix;
	static std::string s_createCDBProjectPrefix;
	static std::string s_createCDBPrefix;
	static std::string s_pingPrefix;
};

#endif	  // NETWORK_PROTOCOL_HELPER_H