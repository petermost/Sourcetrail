#ifndef QT_RESOURCES_H
#define QT_RESOURCES_H

#include <QString>

class QtResources final
{
public:
	static const char ABOUT_CSS[];
	static const char ABOUT_LOGO_SOURCETRAIL[];

	static const char BOOKMARK_VIEW_CSS[];
	static const char BOOKMARK_VIEW_ARROW_DOWN[];
	static const char BOOKMARK_VIEW_ARROW_LINE_DOWN[];
	static const char BOOKMARK_VIEW_ARROW_LINE_UP[];
	static const char BOOKMARK_VIEW_ARROW_RIGHT[];
	static const char BOOKMARK_VIEW_BOOKMARK_ACTIVE[];
	static const char BOOKMARK_VIEW_BOOKMARK_DELETE_ICON[];
	static const char BOOKMARK_VIEW_BOOKMARK_EDIT_ICON[];
	static const char BOOKMARK_VIEW_BOOKMARK_LIST_ICON[];
	static const char BOOKMARK_VIEW_BOOKMARK_EDIT_BOOKMARK_ICON[];

	static const char CODE_VIEW_CSS[];
	static const char CODE_VIEW_ARROW_DOWN[];
	static const char CODE_VIEW_ARROW_LEFT[];
	static const char CODE_VIEW_ARROW_RIGHT[];
	static const char CODE_VIEW_ARROW_UP[];
	static const char CODE_VIEW_BUTTON_BACKGROUND[];
	static const char CODE_VIEW_EDIT[];
	static const char CODE_VIEW_FILE[];
	static const char CODE_VIEW_LIST[];
	static const char CODE_VIEW_MAXIMIZE_ACTIVE[];
	static const char CODE_VIEW_MAXIMIZE_INACTIVE[];
	static const char CODE_VIEW_PATTERN_BRIGHT[];
	static const char CODE_VIEW_PATTERN_DARK[];
	static const char CODE_VIEW_PATTERN_GREY[];
	static const char CODE_VIEW_SNIPPET_ACTIVE[];
	static const char CODE_VIEW_SNIPPET_ARROW_DOWN[];
	static const char CODE_VIEW_SNIPPET_ARROW_RIGHT[];
	static const char CODE_VIEW_SNIPPET_INACTIVE[];

	static const char CUSTOM_TRAIL_VIEW_CSS[];
	static const char CUSTOM_TRAIL_VIEW_CIRCLE[];

	static const char GRAPH_VIEW_ANNOTATION[];
	static const char GRAPH_VIEW_ARROW[];
	static const char GRAPH_VIEW_BUNDLE[];
	static const char GRAPH_VIEW_DEFAULT[];
	static const char GRAPH_VIEW_ENUM[];
	static const char GRAPH_VIEW_FILE_INCOMPLETE[];
	static const char GRAPH_VIEW_FILE[];
	static const char GRAPH_VIEW_GRAPH_ARROW[];
	static const char GRAPH_VIEW_GRAPH_CUSTOM[];
	static const char GRAPH_VIEW_GRAPH_DOWN[];
	static const char GRAPH_VIEW_GRAPH_LEFT[];
	static const char GRAPH_VIEW_GRAPH[];
	static const char GRAPH_VIEW_GRAPH_RIGHT[];
	static const char GRAPH_VIEW_GRAPH_UP[];
	static const char GRAPH_VIEW_GROUP_NAMESPACE[];
	static const char GRAPH_VIEW_LEGEND[];
	static const char GRAPH_VIEW_MACRO[];
	static const char GRAPH_VIEW_NAMESPACE[];
	static const char GRAPH_VIEW_PATTERN[];
	static const char GRAPH_VIEW_PRIVATE[];
	static const char GRAPH_VIEW_PROTECTED[];
	static const char GRAPH_VIEW_PUBLIC[];
	static const char GRAPH_VIEW_TEMPLATE[];
	static const char GRAPH_VIEW_TYPEDEF[];
	static const char GRAPH_VIEW_ZOOM_IN[];
	static const char GRAPH_VIEW_ZOOM_OUT[];
	static const char GRAPH_VIEW_CSS[];

	static const char HISTORY_LIST_CSS[];
	static const char HISTORY_LIST_ARROW[];

	static const char ICON_CBP_ICON[];
	static const char ICON_CDB_ICON[];
	static const char ICON_C_ICON[];
	static const char ICON_CPP_ICON[];
	static const char ICON_EMPTY_ICON[];
	static const char ICON_GRADLE_ICON[];
	static const char ICON_JAVA_ICON[];
	static const char ICON_LOGO_1024_1024[];
	static const char ICON_MVN_ICON[];
	static const char ICON_PROJECT_256_256[];
	static const char ICON_PROJECT[];
	static const char ICON_VS_ICON[];

	static const char INDEXING_DIALOG_CSS[];
	static const char INDEXING_DIALOG_ERROR[];
	static const char INDEXING_DIALOG_FLAG[];
	static const char INDEXING_DIALOG_PROGRESS_BAR_ELEMENT[];

	static const char KEYBOARD_SHORTCUTS_CSS[];

	static const char MAIN_CLOSE_BLACK[];
	static const char MAIN_CLOSE_GREY[];
	static const char MAIN_CLOSE_WHITE[];
	static const char MAIN_FLOAT_BLACK[];
	static const char MAIN_FLOAT_GREY[];
	static const char MAIN_FLOAT_WHITE[];
	static const char MAIN_CSS[];
	static const char MAIN_SCROLLBAR_CSS[];

	static const char REFRESH_VIEW_CSS[];
	static const char REFRESH_VIEW_REFRESH[];

	static const char SCREEN_SEARCH_VIEW_CLOSE[];
	static const char SCREEN_SEARCH_VIEW_CSS[];

	static const char SEARCH_VIEW_ARROW[];
	static const char SEARCH_VIEW_HOME[];
	static const char SEARCH_VIEW_SEARCH[];
	static const char SEARCH_VIEW_CSS[];

	static const char STARTSCREEN_CSS[];
	static const char STARTSCREEN_DELETE_ICON[];
	static const char STARTSCREEN_GITHUB_ICON[];

	static const char STATUSBAR_VIEW_DOT[];
	static const char STATUSBAR_VIEW_LOADER[];

	static const char TABBED_VIEW_ARROW_DOWN[];
	static const char TABBED_VIEW_ARROW_UP[];
	static const char TABBED_VIEW_CSS[];

	static const char TABS_VIEW_ADD[];
	static const char TABS_VIEW_CLOSE[];
	static const char TABS_VIEW_CSS[];

	static const char TOOLTIP_VIEW_CSS[];

	static const char UNDOREDO_VIEW_ARROW_LEFT[];
	static const char UNDOREDO_VIEW_ARROW_RIGHT[];
	static const char UNDOREDO_VIEW_HISTORY[];
	static const char UNDOREDO_VIEW_CSS[];

	static const char WINDOW_DOTS[];
	static const char WINDOW_DOTS_HOVER[];
	static const char WINDOW_HELP_HOVER[];
	static const char WINDOW_HELP[];
	static const char WINDOW_LISTBOX_CSS[];
	static const char WINDOW_LOGO[];
	static const char WINDOW_MINUS_HOVER[];
	static const char WINDOW_MINUS[];
	static const char WINDOW_PLUS_HOVER[];
	static const char WINDOW_PLUS[];
	static const char WINDOW_REFRESH_HOVER[];
	static const char WINDOW_REFRESH[];
	static const char WINDOW_SIZE_GRIP_BLACK[];
	static const char WINDOW_SIZE_GRIP_WHITE[];
	static const char WINDOW_SOURCE_GROUP_ADD_HOVER[];
	static const char WINDOW_SOURCE_GROUP_ADD[];
	static const char WINDOW_SOURCE_GROUP_COPY_HOVER[];
	static const char WINDOW_SOURCE_GROUP_COPY[];
	static const char WINDOW_SOURCE_GROUP_DELETE_HOVER[];
	static const char WINDOW_SOURCE_GROUP_DELETE[];
	static const char WINDOW_CSS[];

	static QString loadStyleSheet(const QString &styleSheetPath);
	static QString loadStyleSheets(const QString &styleSheetPath1, const QString &styleSheetPath2);
};

#endif
