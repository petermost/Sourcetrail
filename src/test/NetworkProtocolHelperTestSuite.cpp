#include "Catch2.hpp"

#include <sstream>

#include "NetworkProtocolHelper.h"

TEST_CASE("parse message")
{
	std::string type = "setActiveToken";
	std::string divider = ">>";
	std::string filePath = "C:/Users/Manuel/important/file/location/fileName.cpp";
	std::string endOfMessageToken = "<EOM>";
	unsigned int row = 1;
	unsigned int column = 2;

	// valid message
	std::stringstream message;
	message << type << divider << filePath << divider << row << divider << column
			<< endOfMessageToken;

	NetworkProtocolHelper::SetActiveTokenMessage networkMessage =
		NetworkProtocolHelper::parseSetActiveTokenMessage(message.str());

	REQUIRE(networkMessage.filePath.str() == filePath);
	REQUIRE(networkMessage.row == row);
	REQUIRE(networkMessage.column == column);
	REQUIRE(networkMessage.valid == true);

	// invalid type
	message.str("");
	message << "foo" << divider << filePath << divider << row << divider << column
			<< endOfMessageToken;
	networkMessage = NetworkProtocolHelper::parseSetActiveTokenMessage(message.str());

	REQUIRE(networkMessage.filePath.str() == "");
	REQUIRE(networkMessage.row == 0);
	REQUIRE(networkMessage.column == 0);
	REQUIRE(networkMessage.valid == false);

	// missing divider
	message.str("");
	message << type << divider << filePath << row << divider << column << endOfMessageToken;
	networkMessage = NetworkProtocolHelper::parseSetActiveTokenMessage(message.str());

	REQUIRE(networkMessage.filePath.str() == "");
	REQUIRE(networkMessage.row == 0);
	REQUIRE(networkMessage.column == 0);
	REQUIRE(networkMessage.valid == false);

	// invalid row
	message.str("");
	message << type << divider << filePath << divider << "potato" << divider << column
			<< endOfMessageToken;
	networkMessage = NetworkProtocolHelper::parseSetActiveTokenMessage(message.str());

	REQUIRE(networkMessage.filePath.str() == "");
	REQUIRE(networkMessage.row == 0);
	REQUIRE(networkMessage.column == 0);
	REQUIRE(networkMessage.valid == false);

	// invalid column
	message.str("");
	message << type << divider << filePath << divider << row << divider << "laz0r"
			<< endOfMessageToken;
	networkMessage = NetworkProtocolHelper::parseSetActiveTokenMessage(message.str());

	REQUIRE(networkMessage.filePath.str() == "");
	REQUIRE(networkMessage.row == 0);
	REQUIRE(networkMessage.column == 0);
	REQUIRE(networkMessage.valid == false);
}
