#ifndef MESSAGE_LOAD_PROJECT_H
#define MESSAGE_LOAD_PROJECT_H

#include "RefreshInfo.h"

#include "FilePath.h"
#include "Message.h"
#include "utilityEnum.h"

class MessageLoadProject: public Message<MessageLoadProject>
{
public:
	MessageLoadProject(
		const FilePath& filePath,
		bool settingsChanged = false,
		RefreshMode refreshMode = RefreshMode::NONE)
		: projectSettingsFilePath(filePath)
		, settingsChanged(settingsChanged)
		, refreshMode(refreshMode)
	{
	}

	static const std::string getStaticType()
	{
		return "MessageLoadProject";
	}

	void print(std::ostream& os) const override
	{
		os << projectSettingsFilePath.str();
		os << ", settingsChanged: " << std::boolalpha << settingsChanged;
		os << ", refreshMode: " << refreshMode;
	}

	const FilePath projectSettingsFilePath;
	const bool settingsChanged;
	const RefreshMode refreshMode;
};

#endif	  // MESSAGE_LOAD_PROJECT_H
