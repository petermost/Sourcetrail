#include "MessageStatus.h"

#include "utilityString.h"

MessageStatus::MessageStatus(
	const std::string& status, bool isError, bool showLoader, bool showInStatusBar)
	: isError(isError), showLoader(showLoader), showInStatusBar(showInStatusBar)
{
	m_stati.push_back(utility::replace(status, "\n", " "));

	setSendAsTask(false);
}

MessageStatus::MessageStatus(
	const std::vector<std::string>& stati, bool isError, bool showLoader, bool showInStatusBar)
	: isError(isError), showLoader(showLoader), showInStatusBar(showInStatusBar), m_stati(stati)
{
	setSendAsTask(false);
}

const std::string MessageStatus::getStaticType()
{
	return "MessageStatus";
}

const std::vector<std::string>& MessageStatus::stati() const
{
	return m_stati;
}

std::string MessageStatus::status() const
{
	if (m_stati.size())
	{
		return m_stati[0];
	}

	return "";
}

void MessageStatus::print(std::ostream& os) const
{
	for (const std::string& status: m_stati)
	{
		os << status;

		if (m_stati.size() > 1)
		{
			os << " - ";
		}
	}

	if (isError)
	{
		os << " - error";
	}

	if (showLoader)
	{
		os << " - loading";
	}
}
