#include "QtResources.h"

#include "ApplicationSettings.h"
#include "ColorScheme.h"
#include "Platform.h"
#include "logging.h"
#include "utilityString.h"

#include <QFile>

#include <vector>

using namespace std;
using namespace utility;

const char QtResources::ABOUT_CSS[]              = ":/about/about.css";
const char QtResources::ABOUT_LOGO_SOURCETRAIL[] = ":/about/logo_sourcetrail.png";

const char QtResources::BOOKMARK_VIEW_CSS[]                         = ":/bookmark_view/bookmark_view.css";
const char QtResources::BOOKMARK_VIEW_ARROW_DOWN[]                  = ":/bookmark_view/images/arrow_down.png";
const char QtResources::BOOKMARK_VIEW_ARROW_LINE_DOWN[]             = ":/bookmark_view/images/arrow_line_down.png";
const char QtResources::BOOKMARK_VIEW_ARROW_LINE_UP[]               = ":/bookmark_view/images/arrow_line_up.png";
const char QtResources::BOOKMARK_VIEW_ARROW_RIGHT[]                 = ":/bookmark_view/images/arrow_right.png";
const char QtResources::BOOKMARK_VIEW_BOOKMARK_ACTIVE[]             = ":/bookmark_view/images/bookmark_active.png";
const char QtResources::BOOKMARK_VIEW_BOOKMARK_DELETE_ICON[]        = ":/bookmark_view/images/bookmark_delete_icon.png";
const char QtResources::BOOKMARK_VIEW_BOOKMARK_EDIT_ICON[]          = ":/bookmark_view/images/bookmark_edit_icon.png";
const char QtResources::BOOKMARK_VIEW_BOOKMARK_LIST_ICON[]          = ":/bookmark_view/images/bookmark_list_icon.png";
const char QtResources::BOOKMARK_VIEW_BOOKMARK_EDIT_BOOKMARK_ICON[] = ":/bookmark_view/images/edit_bookmark_icon.png";

const char QtResources::CODE_VIEW_CSS[]                 = ":/code_view/code_view.css";
const char QtResources::CODE_VIEW_ARROW_DOWN[]          = ":/code_view/images/arrow_down.png";
const char QtResources::CODE_VIEW_ARROW_LEFT[]          = ":/code_view/images/arrow_left.png";
const char QtResources::CODE_VIEW_ARROW_RIGHT[]         = ":/code_view/images/arrow_right.png";
const char QtResources::CODE_VIEW_ARROW_UP[]            = ":/code_view/images/arrow_up.png";
const char QtResources::CODE_VIEW_BUTTON_BACKGROUND[]   = ":/code_view/images/button_background.png";
const char QtResources::CODE_VIEW_EDIT[]                = ":/code_view/images/edit.png";
const char QtResources::CODE_VIEW_FILE[]                = ":/code_view/images/file.png";
const char QtResources::CODE_VIEW_LIST[]                = ":/code_view/images/list.png";
const char QtResources::CODE_VIEW_MAXIMIZE_ACTIVE[]     = ":/code_view/images/maximize_active.png";
const char QtResources::CODE_VIEW_MAXIMIZE_INACTIVE[]   = ":/code_view/images/maximize_inactive.png";
const char QtResources::CODE_VIEW_PATTERN_BRIGHT[]      = ":/code_view/images/pattern_bright.png";
const char QtResources::CODE_VIEW_PATTERN_DARK[]        = ":/code_view/images/pattern_dark.png";
const char QtResources::CODE_VIEW_PATTERN_GREY[]        = ":/code_view/images/pattern_grey.png";
const char QtResources::CODE_VIEW_SNIPPET_ACTIVE[]      = ":/code_view/images/snippet_active.png";
const char QtResources::CODE_VIEW_SNIPPET_ARROW_DOWN[]  = ":/code_view/images/snippet_arrow_down.png";
const char QtResources::CODE_VIEW_SNIPPET_ARROW_RIGHT[] = ":/code_view/images/snippet_arrow_right.png";
const char QtResources::CODE_VIEW_SNIPPET_INACTIVE[]    = ":/code_view/images/snippet_inactive.png";

const char QtResources::CUSTOM_TRAIL_VIEW_CSS[]    = ":/custom_trail_view/custom_trail_view.css";
const char QtResources::CUSTOM_TRAIL_VIEW_CIRCLE[] = ":/custom_trail_view/images/circle.png";

const char QtResources::GRAPH_VIEW_ANNOTATION[]      = ":/graph_view/images/annotation.png";
const char QtResources::GRAPH_VIEW_ARROW[]           = ":/graph_view/images/arrow.png";
const char QtResources::GRAPH_VIEW_BUNDLE[]          = ":/graph_view/images/bundle.png";
const char QtResources::GRAPH_VIEW_DEFAULT[]         = ":/graph_view/images/default.png";
const char QtResources::GRAPH_VIEW_ENUM[]            = ":/graph_view/images/enum.png";
const char QtResources::GRAPH_VIEW_FILE_INCOMPLETE[] = ":/graph_view/images/file_incomplete.png";
const char QtResources::GRAPH_VIEW_FILE[]            = ":/graph_view/images/file.png";
const char QtResources::GRAPH_VIEW_GRAPH_ARROW[]     = ":/graph_view/images/graph_arrow.png";
const char QtResources::GRAPH_VIEW_GRAPH_CUSTOM[]    = ":/graph_view/images/graph_custom.png";
const char QtResources::GRAPH_VIEW_GRAPH_DOWN[]      = ":/graph_view/images/graph_down.png";
const char QtResources::GRAPH_VIEW_GRAPH_LEFT[]      = ":/graph_view/images/graph_left.png";
const char QtResources::GRAPH_VIEW_GRAPH[]           = ":/graph_view/images/graph.png";
const char QtResources::GRAPH_VIEW_GRAPH_RIGHT[]     = ":/graph_view/images/graph_right.png";
const char QtResources::GRAPH_VIEW_GRAPH_UP[]        = ":/graph_view/images/graph_up.png";
const char QtResources::GRAPH_VIEW_GROUP_NAMESPACE[] = ":/graph_view/images/group_namespace.png";
const char QtResources::GRAPH_VIEW_LEGEND[]          = ":/graph_view/images/legend.png";
const char QtResources::GRAPH_VIEW_MACRO[]           = ":/graph_view/images/macro.png";
const char QtResources::GRAPH_VIEW_NAMESPACE[]       = ":/graph_view/images/namespace.png";
const char QtResources::GRAPH_VIEW_PATTERN[]         = ":/graph_view/images/pattern.png";
const char QtResources::GRAPH_VIEW_PRIVATE[]         = ":/graph_view/images/private.png";
const char QtResources::GRAPH_VIEW_PROTECTED[]       = ":/graph_view/images/protected.png";
const char QtResources::GRAPH_VIEW_PUBLIC[]          = ":/graph_view/images/public.png";
const char QtResources::GRAPH_VIEW_TEMPLATE[]        = ":/graph_view/images/template.png";
const char QtResources::GRAPH_VIEW_TYPEDEF[]         = ":/graph_view/images/typedef.png";
const char QtResources::GRAPH_VIEW_ZOOM_IN[]         = ":/graph_view/images/zoom_in.png";
const char QtResources::GRAPH_VIEW_ZOOM_OUT[]        = ":/graph_view/images/zoom_out.png";
const char QtResources::GRAPH_VIEW_CSS[]             = ":/graph_view/graph_view.css";

const char QtResources::HISTORY_LIST_CSS[]   = ":/history_list/history_list.css";
const char QtResources::HISTORY_LIST_ARROW[] = ":/history_list/images/arrow.png";

const char QtResources::ICON_CBP_ICON[]        = ":/icon/cbp_icon.png";
const char QtResources::ICON_CDB_ICON[]        = ":/icon/cdb_icon.png";
const char QtResources::ICON_C_ICON[]          = ":/icon/c_icon.png";
const char QtResources::ICON_CPP_ICON[]        = ":/icon/cpp_icon.png";
const char QtResources::ICON_EMPTY_ICON[]      = ":/icon/empty_icon.png";
const char QtResources::ICON_GRADLE_ICON[]     = ":/icon/gradle_icon.png";
const char QtResources::ICON_JAVA_ICON[]       = ":/icon/java_icon.png";
const char QtResources::ICON_LOGO_1024_1024[]  = ":/icon/logo_1024_1024.png";
const char QtResources::ICON_MVN_ICON[]        = ":/icon/mvn_icon.png";
const char QtResources::ICON_PROJECT_256_256[] = ":/icon/project_256_256.png";
const char QtResources::ICON_PROJECT[]         = ":/icon/project.ico";
const char QtResources::ICON_VS_ICON[]         = ":/icon/vs_icon.png";

const char QtResources::INDEXING_DIALOG_CSS[]                  = ":/indexing_dialog/indexing_dialog.css";
const char QtResources::INDEXING_DIALOG_ERROR[]                = ":/indexing_dialog/error.png";
const char QtResources::INDEXING_DIALOG_FLAG[]                 = ":/indexing_dialog/flag.png";
const char QtResources::INDEXING_DIALOG_PROGRESS_BAR_ELEMENT[] = ":/indexing_dialog/progress_bar_element.png";

const char QtResources::KEYBOARD_SHORTCUTS_CSS[] = ":/keyboard_shortcuts/keyboard_shortcuts.css";

const char QtResources::MAIN_CLOSE_BLACK[]   = ":/main/images/close_black.png";
const char QtResources::MAIN_CLOSE_GREY[]    = ":/main/images/close_grey.png";
const char QtResources::MAIN_CLOSE_WHITE[]   = ":/main/images/close_white.png";
const char QtResources::MAIN_FLOAT_BLACK[]   = ":/main/images/float_black.png";
const char QtResources::MAIN_FLOAT_GREY[]    = ":/main/images/float_grey.png";
const char QtResources::MAIN_FLOAT_WHITE[]   = ":/main/images/float_white.png";
const char QtResources::MAIN_CSS[]           = ":/main/main.css";
const char QtResources::MAIN_SCROLLBAR_CSS[] = ":/main/scrollbar.css";

const char QtResources::REFRESH_VIEW_CSS[]     = ":/refresh_view/refresh_view.css";
const char QtResources::REFRESH_VIEW_REFRESH[] = ":/refresh_view/images/refresh.png";

const char QtResources::SCREEN_SEARCH_VIEW_CLOSE[] = ":/screen_search_view/images/close.png";
const char QtResources::SCREEN_SEARCH_VIEW_CSS[]   = ":/screen_search_view/screen_search_view.css";

const char QtResources::SEARCH_VIEW_ARROW[]  = ":/search_view/images/arrow.png";
const char QtResources::SEARCH_VIEW_HOME[]   = ":/search_view/images/home.png";
const char QtResources::SEARCH_VIEW_SEARCH[] = ":/search_view/images/search.png";
const char QtResources::SEARCH_VIEW_CSS[]    = ":/search_view/search_view.css";

const char QtResources::STARTSCREEN_CSS[]         = ":/startscreen/startscreen.css";
const char QtResources::STARTSCREEN_DELETE_ICON[] = ":/startscreen/delete_icon.png";
const char QtResources::STARTSCREEN_GITHUB_ICON[] = ":/startscreen/github_icon.png";

const char QtResources::STATUSBAR_VIEW_DOT[]    = ":/statusbar_view/dot.png";
const char QtResources::STATUSBAR_VIEW_LOADER[] = ":/statusbar_view/loader.gif";

const char QtResources::TABBED_VIEW_ARROW_DOWN[] = ":/tabbed_view/arrow_down.png";
const char QtResources::TABBED_VIEW_ARROW_UP[]   = ":/tabbed_view/arrow_up.png";
const char QtResources::TABBED_VIEW_CSS[]        = ":/tabbed_view/tabbed_view.css";

const char QtResources::TABS_VIEW_ADD[]   = ":/tabs_view/images/add.png";
const char QtResources::TABS_VIEW_CLOSE[] = ":/tabs_view/images/close.png";
const char QtResources::TABS_VIEW_CSS[]   = ":/tabs_view/tabs_view.css";

const char QtResources::TOOLTIP_VIEW_CSS[] = ":/tooltip_view/tooltip_view.css";

const char QtResources::UNDOREDO_VIEW_ARROW_LEFT[]  = ":/undoredo_view/images/arrow_left.png";
const char QtResources::UNDOREDO_VIEW_ARROW_RIGHT[] = ":/undoredo_view/images/arrow_right.png";
const char QtResources::UNDOREDO_VIEW_HISTORY[]     = ":/undoredo_view/images/history.png";
const char QtResources::UNDOREDO_VIEW_CSS[]         = ":/undoredo_view/undoredo_view.css";

const char QtResources::WINDOW_DOTS[]                      = ":/window/dots.png";
const char QtResources::WINDOW_DOTS_HOVER[]                = ":/window/dots_hover.png";
const char QtResources::WINDOW_HELP_HOVER[]                = ":/window/help_hover.png";
const char QtResources::WINDOW_HELP[]                      = ":/window/help.png";
const char QtResources::WINDOW_LISTBOX_CSS[]               = ":/window/listbox.css";
const char QtResources::WINDOW_LOGO[]                      = ":/window/logo.png";
const char QtResources::WINDOW_MINUS_HOVER[]               = ":/window/minus_hover.png";
const char QtResources::WINDOW_MINUS[]                     = ":/window/minus.png";
const char QtResources::WINDOW_PLUS_HOVER[]                = ":/window/plus_hover.png";
const char QtResources::WINDOW_PLUS[]                      = ":/window/plus.png";
const char QtResources::WINDOW_REFRESH_HOVER[]             = ":/window/refresh_hover.png";
const char QtResources::WINDOW_REFRESH[]                   = ":/window/refresh.png";
const char QtResources::WINDOW_SIZE_GRIP_BLACK[]           = ":/window/size_grip_black.png";
const char QtResources::WINDOW_SIZE_GRIP_WHITE[]           = ":/window/size_grip_white.png";
const char QtResources::WINDOW_SOURCE_GROUP_ADD_HOVER[]    = ":/window/source_group_add_hover.png";
const char QtResources::WINDOW_SOURCE_GROUP_ADD[]          = ":/window/source_group_add.png";
const char QtResources::WINDOW_SOURCE_GROUP_COPY_HOVER[]   = ":/window/source_group_copy_hover.png";
const char QtResources::WINDOW_SOURCE_GROUP_COPY[]         = ":/window/source_group_copy.png";
const char QtResources::WINDOW_SOURCE_GROUP_DELETE_HOVER[] = ":/window/source_group_delete_hover.png";
const char QtResources::WINDOW_SOURCE_GROUP_DELETE[]       = ":/window/source_group_delete.png";
const char QtResources::WINDOW_CSS[]                       = ":/window/window.css";

static string readFile(const QString &path)
{
	QFile file(path);
	if (file.open(QFile::ReadOnly | QFile::Text))
		return file.readAll().toStdString();
	else
	{
		LOG_ERROR("Resource file not found: " + path.toStdString());
		return "";
	}
}

QString QtResources::loadStyleSheet(const QString &path)
{
	string css = readFile(path);

	size_t currentPos = 0;
	while (currentPos < css.length()) // Previous condition 'currentPos != string::npos' seems wrong!
	{
		// Look for tags '<key:value>':

		const size_t leftBracketPos = css.find('<', currentPos);
		const size_t rightBracketPos = css.find('>', currentPos);

		if (leftBracketPos == string::npos || rightBracketPos == string::npos)
		{
			break;
		}
		vector<string> keyValuePair = splitToVector(css.substr(leftBracketPos + 1, rightBracketPos - leftBracketPos - 1), ':');
		if (keyValuePair.size() != 2)
		{
			LOG_ERROR("Syntax error in file: " + path.toStdString());
			return "";
		}
		const string key = keyValuePair.front();
		string value = keyValuePair.back();

		if (key == "setting")
		{
			if (value.find("font_size") != string::npos)
			{
				// check for modifier
				if (value.find('+') != string::npos)
				{
					const size_t findPos = value.find('+');
					string sub = value.substr(findPos + 1);
					int modifier = stoi(sub);
					value = to_string(ApplicationSettings::getInstance()->getFontSize() + modifier);
				}
				else if (value.find('-') != string::npos)
				{
					const size_t findPos = value.find('-');
					string sub = value.substr(findPos + 1);
					int modifier = stoi(sub);
					value = to_string(ApplicationSettings::getInstance()->getFontSize() - modifier);
				}
				else if (value.find('*') != string::npos)
				{
					const size_t findPos = value.find('*');
					string sub = value.substr(findPos + 1);
					int modifier = stoi(sub);
					value = to_string(ApplicationSettings::getInstance()->getFontSize() * modifier);
				}
				else if (value.find('/') != string::npos)
				{
					const size_t findPos = value.find('/');
					string sub = value.substr(findPos + 1);
					int modifier = stoi(sub);
					value = to_string(ApplicationSettings::getInstance()->getFontSize() / modifier);
				}
				else
				{
					value = to_string(ApplicationSettings::getInstance()->getFontSize());
				}
			}
			else if (value == "font_name")
			{
				value = ApplicationSettings::getInstance()->getFontName();
			}
			else if (value == "gui_path")
			{
				value = ":/";
			}
			else
			{
				LOG_ERROR("Syntax error in file: " + path.toStdString());
				return "";
			}
		}
		else if (key == "color")
		{
			if (!ColorScheme::getInstance()->hasColor(value))
			{
				LOG_WARNING("Color scheme does not provide value for key \"" + value + "\" requested by style \"" + path.toStdString() + "\".");
			}
			value = ColorScheme::getInstance()->getColor(value);
		}
		else if (key == "platform_wml")
		{
			vector<string> windowsMacLinuxTriplet = splitToVector(value, '|');
			if (windowsMacLinuxTriplet.size() != 3)
			{
				LOG_ERROR("Syntax error in file: " + path.toStdString());
				return "";
			}
			if constexpr (Platform::isWindows())
				value = windowsMacLinuxTriplet[0];
			else if constexpr (Platform::isMac())
				value = windowsMacLinuxTriplet[1];
			else if constexpr (Platform::isLinux())
				value = windowsMacLinuxTriplet[2];
		}
		else
		{
			LOG_ERROR("Syntax error in file: " + path.toStdString());
			return "";
		}
		css.replace(leftBracketPos, rightBracketPos - leftBracketPos + 1, value);
		currentPos = leftBracketPos + value.size();
	}
	return QString::fromStdString(css);
}

QString QtResources::loadStyleSheets(const QString &styleSheetPath1, const QString &styleSheetPath2)
{
	return loadStyleSheet(styleSheetPath1) % loadStyleSheet(styleSheetPath2);
}
