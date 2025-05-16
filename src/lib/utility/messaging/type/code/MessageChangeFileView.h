#ifndef MESSAGE_CHANGE_FILE_VIEW_H
#define MESSAGE_CHANGE_FILE_VIEW_H

#include "CodeScrollParams.h"
#include "CodeSnippetParams.h"
#include "FilePath.h"
#include "Message.h"
#include "TabIds.h"

class MessageChangeFileView: public Message<MessageChangeFileView>
{
public:
	enum FileState
	{
		FILE_MINIMIZED,
		FILE_SNIPPETS,
		FILE_MAXIMIZED
	};

	enum ViewMode
	{
		VIEW_LIST,
		VIEW_SINGLE,
		VIEW_CURRENT
	};

	MessageChangeFileView(
		const FilePath& filePath,
		FileState state,
		ViewMode viewMode,
		CodeScrollParams scrollParams,
		bool switchesViewMode = false)
		: filePath(filePath)
		, state(state)
		, viewMode(viewMode)
		, scrollParams(scrollParams)
		, switchesViewMode(switchesViewMode)
	{
		setSchedulerId(TabIds::currentTab());
	}

	static const std::string getStaticType()
	{
		return "MessageChangeFileView";
	}

	void print(std::ostream& os) const override
	{
		os << filePath.str();

		switch (state)
		{
		case FILE_MINIMIZED:
			os << ", minimize";
			break;
		case FILE_SNIPPETS:
			os << ", snippets";
			break;
		case FILE_MAXIMIZED:
			os << ", maximize";
			break;
		}

		switch (viewMode)
		{
		case VIEW_LIST:
			os << ", list";
			break;
		case VIEW_SINGLE:
			os << ", single";
			break;
		case VIEW_CURRENT:
			os << ", current";
			break;
		}
	}

	const FilePath filePath;
	const FileState state;
	const ViewMode viewMode;
	const CodeScrollParams scrollParams;
	const bool switchesViewMode;
};

#endif	  // MESSAGE_CHANGE_FILE_VIEW_H
