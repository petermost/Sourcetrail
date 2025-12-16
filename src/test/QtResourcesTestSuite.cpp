#include "QtResources.h"
#include "FilePath.h"

#include "Catch2.hpp"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>

using namespace std;

namespace
{

const char *resourceNames[] = {
	QtResources::ABOUT_CSS,
	QtResources::ABOUT_LOGO_SOURCETRAIL,

	QtResources::BOOKMARK_VIEW_CSS,
	QtResources::BOOKMARK_VIEW_ARROW_DOWN,
	QtResources::BOOKMARK_VIEW_ARROW_LINE_DOWN,
	QtResources::BOOKMARK_VIEW_ARROW_LINE_UP,
	QtResources::BOOKMARK_VIEW_ARROW_RIGHT,
	QtResources::BOOKMARK_VIEW_BOOKMARK_ACTIVE,
	QtResources::BOOKMARK_VIEW_BOOKMARK_DELETE_ICON,
	QtResources::BOOKMARK_VIEW_BOOKMARK_EDIT_ICON,
	QtResources::BOOKMARK_VIEW_BOOKMARK_LIST_ICON,
	QtResources::BOOKMARK_VIEW_BOOKMARK_EDIT_BOOKMARK_ICON,

	QtResources::CODE_VIEW_CSS,
	QtResources::CODE_VIEW_ARROW_DOWN,
	QtResources::CODE_VIEW_ARROW_LEFT,
	QtResources::CODE_VIEW_ARROW_RIGHT,
	QtResources::CODE_VIEW_ARROW_UP,
	QtResources::CODE_VIEW_BUTTON_BACKGROUND,
	QtResources::CODE_VIEW_EDIT,
	QtResources::CODE_VIEW_FILE,
	QtResources::CODE_VIEW_LIST,
	QtResources::CODE_VIEW_MAXIMIZE_ACTIVE,
	QtResources::CODE_VIEW_MAXIMIZE_INACTIVE,
	QtResources::CODE_VIEW_PATTERN_BRIGHT,
	QtResources::CODE_VIEW_PATTERN_DARK,
	QtResources::CODE_VIEW_PATTERN_GREY,
	QtResources::CODE_VIEW_SNIPPET_ACTIVE,
	QtResources::CODE_VIEW_SNIPPET_ARROW_DOWN,
	QtResources::CODE_VIEW_SNIPPET_ARROW_RIGHT,
	QtResources::CODE_VIEW_SNIPPET_INACTIVE,

	QtResources::CUSTOM_TRAIL_VIEW_CSS,
	QtResources::CUSTOM_TRAIL_VIEW_CIRCLE,

	QtResources::GRAPH_VIEW_ANNOTATION,
	QtResources::GRAPH_VIEW_ARROW,
	QtResources::GRAPH_VIEW_BUNDLE,
	QtResources::GRAPH_VIEW_DEFAULT,
	QtResources::GRAPH_VIEW_ENUM,
	QtResources::GRAPH_VIEW_FILE_INCOMPLETE,
	QtResources::GRAPH_VIEW_FILE,
	QtResources::GRAPH_VIEW_GRAPH_ARROW,
	QtResources::GRAPH_VIEW_GRAPH_CUSTOM,
	QtResources::GRAPH_VIEW_GRAPH_DOWN,
	QtResources::GRAPH_VIEW_GRAPH_LEFT,
	QtResources::GRAPH_VIEW_GRAPH,
	QtResources::GRAPH_VIEW_GRAPH_RIGHT,
	QtResources::GRAPH_VIEW_GRAPH_UP,
	QtResources::GRAPH_VIEW_GROUP_NAMESPACE,
	QtResources::GRAPH_VIEW_LEGEND,
	QtResources::GRAPH_VIEW_MACRO,
	QtResources::GRAPH_VIEW_NAMESPACE,
	QtResources::GRAPH_VIEW_PATTERN,
	QtResources::GRAPH_VIEW_PRIVATE,
	QtResources::GRAPH_VIEW_PROTECTED,
	QtResources::GRAPH_VIEW_PUBLIC,
	QtResources::GRAPH_VIEW_TEMPLATE,
	QtResources::GRAPH_VIEW_TYPEDEF,
	QtResources::GRAPH_VIEW_ZOOM_IN,
	QtResources::GRAPH_VIEW_ZOOM_OUT,
	QtResources::GRAPH_VIEW_CSS,

	QtResources::HISTORY_LIST_CSS,
	QtResources::HISTORY_LIST_ARROW,

	QtResources::ICON_CBP_ICON,
	QtResources::ICON_CDB_ICON,
	QtResources::ICON_C_ICON,
	QtResources::ICON_CPP_ICON,
	QtResources::ICON_EMPTY_ICON,
	QtResources::ICON_GRADLE_ICON,
	QtResources::ICON_JAVA_ICON,
	QtResources::ICON_LOGO_1024_1024,
	QtResources::ICON_MVN_ICON,
	QtResources::ICON_PROJECT_256_256,
	QtResources::ICON_PROJECT,
	QtResources::ICON_VS_ICON,

	QtResources::INDEXING_DIALOG_CSS,
	QtResources::INDEXING_DIALOG_ERROR,
	QtResources::INDEXING_DIALOG_FLAG,
	QtResources::INDEXING_DIALOG_PROGRESS_BAR_ELEMENT,

	QtResources::KEYBOARD_SHORTCUTS_CSS,

	QtResources::MAIN_CLOSE_BLACK,
	QtResources::MAIN_CLOSE_GREY,
	QtResources::MAIN_CLOSE_WHITE,
	QtResources::MAIN_FLOAT_BLACK,
	QtResources::MAIN_FLOAT_GREY,
	QtResources::MAIN_FLOAT_WHITE,
	QtResources::MAIN_CSS,
	QtResources::MAIN_SCROLLBAR_CSS,

	QtResources::REFRESH_VIEW_CSS,
	QtResources::REFRESH_VIEW_REFRESH,

	QtResources::SCREEN_SEARCH_VIEW_CLOSE,
	QtResources::SCREEN_SEARCH_VIEW_CSS,

	QtResources::SEARCH_VIEW_ARROW,
	QtResources::SEARCH_VIEW_HOME,
	QtResources::SEARCH_VIEW_SEARCH,
	QtResources::SEARCH_VIEW_CSS,

	QtResources::STARTSCREEN_CSS,
	QtResources::STARTSCREEN_DELETE_ICON,
	QtResources::STARTSCREEN_GITHUB_ICON,

	QtResources::STATUSBAR_VIEW_DOT,
	QtResources::STATUSBAR_VIEW_LOADER,

	QtResources::TABBED_VIEW_ARROW_DOWN,
	QtResources::TABBED_VIEW_ARROW_UP,
	QtResources::TABBED_VIEW_CSS,

	QtResources::TABS_VIEW_ADD,
	QtResources::TABS_VIEW_CLOSE,
	QtResources::TABS_VIEW_CSS,

	QtResources::TOOLTIP_VIEW_CSS,

	QtResources::UNDOREDO_VIEW_ARROW_LEFT,
	QtResources::UNDOREDO_VIEW_ARROW_RIGHT,
	QtResources::UNDOREDO_VIEW_HISTORY,
	QtResources::UNDOREDO_VIEW_CSS,

	QtResources::WINDOW_DOTS,
	QtResources::WINDOW_DOTS_HOVER,
	QtResources::WINDOW_HELP_HOVER,
	QtResources::WINDOW_HELP,
	QtResources::WINDOW_LISTBOX_CSS,
	QtResources::WINDOW_LOGO,
	QtResources::WINDOW_MINUS_HOVER,
	QtResources::WINDOW_MINUS,
	QtResources::WINDOW_PLUS_HOVER,
	QtResources::WINDOW_PLUS,
	QtResources::WINDOW_REFRESH_HOVER,
	QtResources::WINDOW_REFRESH,
	QtResources::WINDOW_SIZE_GRIP_BLACK,
	QtResources::WINDOW_SIZE_GRIP_WHITE,
	QtResources::WINDOW_SOURCE_GROUP_ADD_HOVER,
	QtResources::WINDOW_SOURCE_GROUP_ADD,
	QtResources::WINDOW_SOURCE_GROUP_COPY_HOVER,
	QtResources::WINDOW_SOURCE_GROUP_COPY,
	QtResources::WINDOW_SOURCE_GROUP_DELETE_HOVER,
	QtResources::WINDOW_SOURCE_GROUP_DELETE,
	QtResources::WINDOW_CSS
};

bool resourceExists(const char resourceName[])
{
	return QFileInfo::exists(resourceName);
}

QList<QString> listAllEntries(const QString &directory)
{
	QList<QString> entries;

	QDirIterator directoryIterator(directory, QDir::Files, QDirIterator::Subdirectories);
	while (directoryIterator.hasNext())
	{
		QString entry = directoryIterator.next();
		// Filter out Qt internal directories/resources:
		if (entry.startsWith(":/qpdf") || entry.startsWith(":/qt-project.org") ||
			// vcpkg-build resources:
			entry.startsWith(":/cursor-atlas.png") || entry.startsWith(":/cursor.json"))
		{
			continue;
		}
		entries.append(entry);
	}
	return entries;
}

}

TEST_CASE("resources count")
{
	auto list = listAllEntries(":/");

	REQUIRE(list.count() == static_cast<qsizetype>(size(resourceNames)));
}

TEST_CASE("resources exist")
{
	auto *resourcesBegin(begin(resourceNames));
	auto *resourcesEnd(end(resourceNames));

	while (resourcesBegin != resourcesEnd)
	{
		REQUIRE_MESSAGE(*resourcesBegin, resourceExists(*resourcesBegin));
		resourcesBegin++;
	}
}

TEST_CASE("resources FilePath can handle Qt resource names")
{
	const char resourceName[] = ":/directory/name";

	FilePath filePath(resourceName);

	REQUIRE(filePath.str() == resourceName);
}
