#ifndef MESSAGE_STATUS_H
#define MESSAGE_STATUS_H

#include <string>
#include <vector>

#include "Message.h"

class MessageStatus: public Message<MessageStatus>
{
public:
	MessageStatus(
		const std::string& status,
		bool isError = false,
		bool showLoader = false,
		bool showInStatusBar = true);
	MessageStatus(
		const std::vector<std::string>& stati,
		bool isError = false,
		bool showLoader = false,
		bool showInStatusBar = true);

	static const std::string getStaticType();

	const std::vector<std::string>& stati() const;
	std::string status() const;
	void print(std::ostream& os) const override;

	const bool isError;
	const bool showLoader;
	const bool showInStatusBar;

private:
	std::vector<std::string> m_stati;
};

#endif	  // MESSAGE_STATUS_H
