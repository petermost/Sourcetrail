#include "QtActions.h"

#include <aidkit/qt/Strings.hpp>

#include <QKeyEvent>

using namespace aidkit::qt;

namespace
{

QString join(const QList<QString> &list)
{
	QString str;

	auto it = list.begin();
	if (it != list.end()) {
		str.append(*it++);
		while (it != list.end())
			str.append(" | ").append(*it++);
	}
	return str;
}

class QtInputCombination
{
public:
	virtual ~QtInputCombination() = default;

	virtual operator QString() const = 0;

protected:
	static QString toString(const QKeySequence &sequence)
	{
		return sequence.toString(QKeySequence::SequenceFormat::NativeText);
	}
};

class QtKeyCombination final : public QtInputCombination
{
public:
	constexpr explicit QtKeyCombination(const Qt::Key key, const Qt::KeyboardModifiers modifiers = Qt::NoModifier) noexcept
		: m_combination(modifiers, key)
	{
	}

	operator QString() const override
	{
		return toString(QKeySequence(m_combination));
	}

	constexpr operator int() const noexcept
	{
		return m_combination.toCombined();
	}
private:
	QKeyCombination m_combination;
};

class QtStandardKeyCombination final : public QtInputCombination
{
public:
	constexpr explicit QtStandardKeyCombination(const QKeySequence::StandardKey key, const Qt::KeyboardModifiers modifiers = Qt::NoModifier) noexcept
		: m_key(key), m_modifiers(modifiers)
	{
	}

	operator QString() const override
	{
		QKeySequence keySequence(m_key);

		if (!keySequence.isEmpty())
			keySequence = QKeySequence(m_modifiers | keySequence[0].keyboardModifiers() | keySequence[0].key());

		return toString(keySequence);
	}

private:
	QKeySequence::StandardKey m_key;
	Qt::KeyboardModifiers m_modifiers;
};

enum class MouseAction
{
	LeftClick, LeftDoubleClick, MiddleClick, WheelUp, WheelDown
};

class QtMouseActionCombination final : public QtInputCombination
{
public:
	constexpr explicit QtMouseActionCombination(const MouseAction action, const Qt::KeyboardModifiers modifiers = Qt::NoModifier)
		: m_action(action), m_modifiers(modifiers)
	{
	}

	operator QString() const override
	{
		QString action;

		switch (m_action)
		{
			case MouseAction::LeftClick:
				action = "Left Click";
				break;

			case MouseAction::LeftDoubleClick:
				action = "Left Double Click";
				break;

			case MouseAction::MiddleClick:
				action = "Middle Click";
				break;

			case MouseAction::WheelUp:
				action = "Mouse Wheel Up";
				break;

			case MouseAction::WheelDown:
				action = "Mouse Wheel Down";
				break;
		}
		return toString(QKeySequence(m_modifiers)) + action;
	}

private:
	MouseAction m_action;
	Qt::KeyboardModifiers m_modifiers;
};

constexpr Qt::KeyboardModifier Alt = Qt::AltModifier;
constexpr Qt::KeyboardModifier Ctrl = Qt::ControlModifier;
constexpr Qt::KeyboardModifier Shift = Qt::ShiftModifier;

// Vim keys:

constexpr Qt::Key KEY_VIM_UP    = Qt::Key_K;
constexpr Qt::Key KEY_VIM_DOWN  = Qt::Key_J;
constexpr Qt::Key KEY_VIM_LEFT  = Qt::Key_H;
constexpr Qt::Key KEY_VIM_RIGHT = Qt::Key_L;

// Game/WASD keys:

constexpr Qt::Key KEY_GAME_UP    = Qt::Key_W;
constexpr Qt::Key KEY_GAME_DOWN  = Qt::Key_S;
constexpr Qt::Key KEY_GAME_LEFT  = Qt::Key_A;
constexpr Qt::Key KEY_GAME_RIGHT = Qt::Key_D;

//
// Can't use constant QKeySequences because the ctor depends on an existing QCoreApplication!
//
// Constants starting with MENU_ or KEYBOARD_ are 'real' key combinations. i.e. Qt can parse them.
// Constants starting with MOUSE_ are 'pseudo' key combinations and Qt can't parse them.
// So only MENU_/KEYBOARD_ constants can be used to initialize a QAction/QKeySequence!
//

// General:

constexpr QtStandardKeyCombination MENU_NEW_PROJECT(QKeySequence::New);
constexpr QtStandardKeyCombination MENU_OPEN_PROJECT(QKeySequence::Open);
constexpr QtStandardKeyCombination MENU_SAVE_AS(QKeySequence::SaveAs);
constexpr QtKeyCombination MENU_EXIT(Qt::Key_Q, Ctrl);

constexpr QtKeyCombination KEYBOARD_CANCEL(Qt::Key_Escape);

constexpr QtKeyCombination KEYBOARD_CUSTOM_TRAIL_DIALOG(Qt::Key_U, Ctrl);

constexpr QtKeyCombination KEYBOARD_TO_OVERVIEW(Qt::Key_Home, Ctrl);

constexpr QtKeyCombination KEYBOARD_PREFERENCES(Qt::Key_Comma, Ctrl);

// Symbol Search:

constexpr QtStandardKeyCombination MENU_FIND_SYMBOL(QKeySequence::Find);
constexpr QtStandardKeyCombination MENU_FIND_TEXT(  QKeySequence::Find, Ctrl | Shift);

constexpr QtKeyCombination KEYBOARD_EXECUTE_SEARCH(Qt::Key_Return);

constexpr QtKeyCombination KEYBOARD_NEXT_MATCH(    Qt::Key_Return);
constexpr QtKeyCombination KEYBOARD_PREVIOUS_MATCH(Qt::Key_Return, Shift);

// Screen Search:

constexpr QtKeyCombination KEYBOARD_SCREEN_SEARCH_1(Qt::Key_D, Ctrl);
constexpr QtKeyCombination KEYBOARD_SCREEN_SEARCH_2(Qt::Key_Slash);
constexpr QtKeyCombination KEYBOARD_SCREEN_SEARCH_3(Qt::Key_Question);

// Help:

constexpr QtKeyCombination KEYBOARD_SHOW_KEYBOARD_SHORTCUTS(Qt::Key_F1);
constexpr QtKeyCombination KEYBOARD_SHOW_LEGEND(            Qt::Key_F1, Shift);

// Zoom:

constexpr QtStandardKeyCombination MENU_LARGER_FONT(QKeySequence::ZoomIn);
constexpr QtStandardKeyCombination MENU_SMALLER_FONT(QKeySequence::ZoomOut);
constexpr QtKeyCombination         MENU_RESET_FONT(Qt::Key_0, Ctrl);

constexpr QtKeyCombination KEYBOARD_ZOOM_IN(Qt::Key_Up, Ctrl | Shift);
constexpr QtKeyCombination KEYBOARD_ZOOM_OUT(Qt::Key_Down, Ctrl | Shift);
constexpr QtKeyCombination KEYBOARD_RESET_ZOOM(Qt::Key_0);

// Navigate References:

constexpr QtKeyCombination KEYBOARD_NEXT_REFERENCE(    Qt::Key_G, Ctrl);
constexpr QtKeyCombination KEYBOARD_PREVIOUS_REFERENCE(Qt::Key_G, Ctrl | Shift);

constexpr QtKeyCombination KEYBOARD_NEXT_LOCAL_REFERENCE(    Qt::Key_L, Ctrl);
constexpr QtKeyCombination KEYBOARD_PREVIOUS_LOCAL_REFERENCE(Qt::Key_L, Ctrl | Shift);

// Move View:

constexpr QtKeyCombination KEYBOARD_MOVE_VIEW_LEFT( Qt::Key_Left, Ctrl);
constexpr QtKeyCombination KEYBOARD_MOVE_VIEW_RIGHT(Qt::Key_Right, Ctrl);
constexpr QtKeyCombination KEYBOARD_MOVE_VIEW_UP(   Qt::Key_Up, Ctrl);
constexpr QtKeyCombination KEYBOARD_MOVE_VIEW_DOWN (Qt::Key_Down, Ctrl);

// Navigate Focus:

constexpr QtKeyCombination KEYBOARD_MOVE_FOCUS_LEFT( Qt::Key_Left);
constexpr QtKeyCombination KEYBOARD_MOVE_FOCUS_RIGHT(Qt::Key_Right);
constexpr QtKeyCombination KEYBOARD_MOVE_FOCUS_UP(   Qt::Key_Up);
constexpr QtKeyCombination KEYBOARD_MOVE_FOCUS_DOWN( Qt::Key_Down);

constexpr QtKeyCombination KEYBOARD_MOVE_VIM_FOCUS_LEFT( KEY_VIM_LEFT);
constexpr QtKeyCombination KEYBOARD_MOVE_VIM_FOCUS_RIGHT(KEY_VIM_RIGHT);
constexpr QtKeyCombination KEYBOARD_MOVE_VIM_FOCUS_UP(   KEY_VIM_UP);
constexpr QtKeyCombination KEYBOARD_MOVE_VIM_FOCUS_DOWN( KEY_VIM_DOWN);

constexpr QtKeyCombination KEYBOARD_MOVE_GAME_FOCUS_LEFT( KEY_GAME_LEFT);
constexpr QtKeyCombination KEYBOARD_MOVE_GAME_FOCUS_RIGHT(KEY_GAME_RIGHT);
constexpr QtKeyCombination KEYBOARD_MOVE_GAME_FOCUS_UP(   KEY_GAME_UP);
constexpr QtKeyCombination KEYBOARD_MOVE_GAME_FOCUS_DOWN( KEY_GAME_DOWN);

constexpr QtKeyCombination KEYBOARD_MOVE_REFERENCE_FOCUS_LEFT( Qt::Key_Left, Shift);
constexpr QtKeyCombination KEYBOARD_MOVE_REFERENCE_FOCUS_RIGHT(Qt::Key_Right, Shift);
constexpr QtKeyCombination KEYBOARD_MOVE_REFERENCE_FOCUS_UP(   Qt::Key_Up, Shift);
constexpr QtKeyCombination KEYBOARD_MOVE_REFERENCE_FOCUS_DOWN( Qt::Key_Down, Shift);

constexpr QtKeyCombination KEYBOARD_MOVE_VIM_REFERENCE_FOCUS_LEFT( KEY_VIM_LEFT, Shift);
constexpr QtKeyCombination KEYBOARD_MOVE_VIM_REFERENCE_FOCUS_RIGHT(KEY_VIM_RIGHT, Shift);
constexpr QtKeyCombination KEYBOARD_MOVE_VIM_REFERENCE_FOCUS_UP(   KEY_VIM_UP, Shift);
constexpr QtKeyCombination KEYBOARD_MOVE_VIM_REFERENCE_FOCUS_DOWN( KEY_VIM_DOWN, Shift);

constexpr QtKeyCombination KEYBOARD_MOVE_GAME_REFERENCE_FOCUS_LEFT( KEY_GAME_LEFT, Shift);
constexpr QtKeyCombination KEYBOARD_MOVE_GAME_REFERENCE_FOCUS_RIGHT(KEY_GAME_RIGHT, Shift);
constexpr QtKeyCombination KEYBOARD_MOVE_GAME_REFERENCE_FOCUS_UP(   KEY_GAME_UP, Shift);
constexpr QtKeyCombination KEYBOARD_MOVE_GAME_REFERENCE_FOCUS_DOWN( KEY_GAME_DOWN, Shift);

// Activate Focus:

constexpr QtKeyCombination KEYBOARD_ACTIVATE_FOCUS_1(Qt::Key_Return);
constexpr QtKeyCombination KEYBOARD_ACTIVATE_FOCUS_2(Qt::Key_E);

constexpr QtKeyCombination KEYBOARD_ACTIVATE_FOCUS_IN_NEW_TAB_1(Qt::Key_Return, Ctrl | Shift);
constexpr QtKeyCombination KEYBOARD_ACTIVATE_FOCUS_IN_NEW_TAB_2(Qt::Key_E, Ctrl | Shift);

constexpr QtKeyCombination KEYBOARD_EXPAND_FOCUS_1(Qt::Key_Return, Shift);
constexpr QtKeyCombination KEYBOARD_EXPAND_FOCUS_2(Qt::Key_E, Shift);

constexpr QtKeyCombination KEYBOARD_SWITCH_GRAPH_CODE_FOCUS(Qt::Key_Tab);

// History:

constexpr QtStandardKeyCombination MENU_HISTORY_UNDO(QKeySequence::Back);
constexpr QtKeyCombination         KEYBOARD_HISTORY_UNDO_1(Qt::Key_Y);
constexpr QtKeyCombination         KEYBOARD_HISTORY_UNDO_2(Qt::Key_Z);
constexpr QtKeyCombination         KEYBOARD_HISTORY_UNDO_3(Qt::Key_Backspace);

constexpr QtStandardKeyCombination MENU_HISTORY_REDO(QKeySequence::Forward);
constexpr QtKeyCombination         KEYBOARD_HISTORY_REDO_1(Qt::Key_Y, Shift);
constexpr QtKeyCombination         KEYBOARD_HISTORY_REDO_2(Qt::Key_Z, Shift);

// Refresh:

constexpr QtStandardKeyCombination MENU_REFRESH(     QKeySequence::Refresh);
constexpr QtStandardKeyCombination MENU_FULL_REFRESH(QKeySequence::Refresh, Shift);

constexpr QtKeyCombination KEYBOARD_REFRESH_UI(Qt::Key_R, Ctrl | Alt);

// Tabs:

constexpr QtStandardKeyCombination MENU_NEW_TAB(QKeySequence::AddTab);

constexpr QtKeyCombination KEYBOARD_SELECT_NEXT_TAB(    Qt::Key_Tab, Ctrl);
constexpr QtKeyCombination KEYBOARD_SELECT_PREVIOUS_TAB(Qt::Key_Tab, Ctrl | Shift);

constexpr QtStandardKeyCombination MENU_CLOSE_TAB(QKeySequence::Close);
constexpr QtKeyCombination         KEYBOARD_CLOSE_TAB(Qt::Key_F4, Ctrl);
constexpr QtMouseActionCombination MOUSE_CLOSE_TAB(MouseAction::MiddleClick);

// Bookmark:

constexpr QtStandardKeyCombination KEYBOARD_BOOKMARK(QKeySequence::Save);
constexpr QtKeyCombination         KEYBOARD_BOOKMARK_MANAGER(Qt::Key_B, Ctrl);

// Clipboard:

constexpr QtKeyCombination KEYBOARD_SAVE_TO_CLIPBOARD(Qt::Key_C, Ctrl);

// Pseudo mouse actions:

constexpr QtMouseActionCombination MOUSE_ACTIVATE_BOOKMARK_1(MouseAction::LeftClick);
constexpr QtMouseActionCombination MOUSE_ACTIVATE_BOOKMARK_2(MouseAction::LeftDoubleClick);

constexpr QtMouseActionCombination MOUSE_SHOW_IN_IDE(MouseAction::LeftClick, Ctrl);

constexpr QtMouseActionCombination MOUSE_ZOOM_IN(MouseAction::WheelUp, Ctrl);
constexpr QtMouseActionCombination MOUSE_ZOOM_OUT(MouseAction::WheelDown, Ctrl);

constexpr QtMouseActionCombination MOUSE_OPEN_IN_NEW_TAB(MouseAction::LeftClick, Ctrl | Shift);
constexpr QtMouseActionCombination MOUSE_SHOW_DEFINITION(MouseAction::LeftClick, Alt | Ctrl);

constexpr QtMouseActionCombination MOUSE_EXPAND(MouseAction::LeftClick, Shift);
constexpr QtMouseActionCombination MOUSE_COLLAPSE(MouseAction::LeftClick, Shift);

constexpr QtMouseActionCombination MOUSE_HIDE(MouseAction::LeftClick, Alt);

} // namespace

///////////////////////////////////////////////////////////////////////////////
//
// QtActions
//
///////////////////////////////////////////////////////////////////////////////

Action QtActions::detectAction(const QKeyEvent *event)
{
	switch (QtKeyCombination(static_cast<Qt::Key>(event->key()), event->modifiers()))
	{
		// Zoom:

		case KEYBOARD_ZOOM_IN:
			return Action::ZoomIn;
		case KEYBOARD_ZOOM_OUT:
			return Action::ZoomOut;
		case KEYBOARD_RESET_ZOOM:
			return Action::ResetZoom;

		// Up:

		case KEYBOARD_MOVE_FOCUS_UP:
		case KEYBOARD_MOVE_VIM_FOCUS_UP:
		case KEYBOARD_MOVE_GAME_FOCUS_UP:
			return Action::MoveFocusUp;

		case KEYBOARD_MOVE_VIEW_UP:
			return Action::MoveViewUp;

		case KEYBOARD_MOVE_REFERENCE_FOCUS_UP:
		case KEYBOARD_MOVE_VIM_REFERENCE_FOCUS_UP:
		case KEYBOARD_MOVE_GAME_REFERENCE_FOCUS_UP:
			return Action::MoveReferenceFocusUp;

		// Down:

		case KEYBOARD_MOVE_FOCUS_DOWN:
		case KEYBOARD_MOVE_VIM_FOCUS_DOWN:
		case KEYBOARD_MOVE_GAME_FOCUS_DOWN:
			return Action::MoveFocusDown;

		case KEYBOARD_MOVE_VIEW_DOWN:
			return Action::MoveViewDown;

		case KEYBOARD_MOVE_REFERENCE_FOCUS_DOWN:
		case KEYBOARD_MOVE_VIM_REFERENCE_FOCUS_DOWN:
		case KEYBOARD_MOVE_GAME_REFERENCE_FOCUS_DOWN:
			return Action::MoveReferenceFocusDown;

		// Left:

		case KEYBOARD_MOVE_FOCUS_LEFT:
		case KEYBOARD_MOVE_VIM_FOCUS_LEFT:
		case KEYBOARD_MOVE_GAME_FOCUS_LEFT:
			return Action::MoveFocusLeft;

		case KEYBOARD_MOVE_VIEW_LEFT:
			return Action::MoveViewLeft;

		case KEYBOARD_MOVE_REFERENCE_FOCUS_LEFT:
		case KEYBOARD_MOVE_VIM_REFERENCE_FOCUS_LEFT:
		case KEYBOARD_MOVE_GAME_REFERENCE_FOCUS_LEFT:
			return Action::MoveReferenceFocusLeft;

		// Right:

		case KEYBOARD_MOVE_FOCUS_RIGHT:
		case KEYBOARD_MOVE_VIM_FOCUS_RIGHT:
		case KEYBOARD_MOVE_GAME_FOCUS_RIGHT:
			return Action::MoveFocusRight;

		case KEYBOARD_MOVE_VIEW_RIGHT:
			return Action::MoveViewRight;

		case KEYBOARD_MOVE_REFERENCE_FOCUS_RIGHT:
		case KEYBOARD_MOVE_VIM_REFERENCE_FOCUS_RIGHT:
		case KEYBOARD_MOVE_GAME_REFERENCE_FOCUS_RIGHT:
			return Action::MoveReferenceFocusRight;

		// Activate Focus:

		case KEYBOARD_ACTIVATE_FOCUS_1:
		case KEYBOARD_ACTIVATE_FOCUS_2:
			return Action::ActivateFocus;

		case KEYBOARD_EXPAND_FOCUS_1:
		case KEYBOARD_EXPAND_FOCUS_2:
			return Action::ExpandFocus;

		case KEYBOARD_ACTIVATE_FOCUS_IN_NEW_TAB_1:
		case KEYBOARD_ACTIVATE_FOCUS_IN_NEW_TAB_2:
			return Action::ActivateFocusInNewTab;

		// Switch Graph/Code:

		case KEYBOARD_SWITCH_GRAPH_CODE_FOCUS:
			return Action::SwitchGraphCodeFocus;

		// Screen search:

		case KEYBOARD_SCREEN_SEARCH_1:
		case KEYBOARD_SCREEN_SEARCH_2:
		case KEYBOARD_SCREEN_SEARCH_3:
			return Action::SearchScreen;

		// History:

		case KEYBOARD_HISTORY_UNDO_1:
		case KEYBOARD_HISTORY_UNDO_2:
		case KEYBOARD_HISTORY_UNDO_3:
			return Action::UndoHistory;

		case KEYBOARD_HISTORY_REDO_1:
		case KEYBOARD_HISTORY_REDO_2:
			return Action::RedoHistory;

		// Clipboard:

		case KEYBOARD_SAVE_TO_CLIPBOARD:
			return Action::CopySelection;

		// Cancel:

		case KEYBOARD_CANCEL:
			return Action::Cancel;

		// Refresh UI:

		case KEYBOARD_REFRESH_UI:
			return Action::RefreshUI;

		// Close Tab:

		case KEYBOARD_CLOSE_TAB:
			return Action::CloseTab;
	}
	return Action::Unknown;
}

QtActions::Info::Info(const QString &text, const QString &shortcut)
	: Info(text, {shortcut})
{
}

QtActions::Info::Info(const QString &text, const std::initializer_list<QString> shortcuts)
	: m_text(text)
	, m_shortcuts(shortcuts)
{
}

QString QtActions::Info::text() const
{
	return m_text;
}

QString QtActions::Info::tooltip() const
{
	return "%1 (%2)"_qs.arg(name(), shortcuts());
}

QString QtActions::Info::menuText() const
{
	// Separate the label and shortcut with '\t' which right-alignes the shortcut:

	return "%1\t%2"_qs.arg(m_text, shortcuts());
}

QString QtActions::Info::name() const
{
	return QString(m_text).remove('&').remove("...");
}

QString QtActions::Info::shortcut() const
{
	return m_shortcuts.first();
}

QString QtActions::Info::shortcuts() const
{
	return join(m_shortcuts);
}




QtActions::Info QtActions::newProject()
{
	return Info(tr("&New Project..."), MENU_NEW_PROJECT);
}

QtActions::Info QtActions::openProject()
{
	return Info(tr("&Open Project..."), MENU_OPEN_PROJECT);
}

QtActions::Info QtActions::exit()
{
	return Info(tr("E&xit"), MENU_EXIT);
}

QtActions::Info QtActions::refresh()
{
	return Info(tr("&Refresh..."), MENU_REFRESH);
}

QtActions::Info QtActions::fullRefresh()
{
	return Info(tr("&Full Refresh..."), MENU_FULL_REFRESH);
}

QtActions::Info QtActions::findSymbol()
{
	return Info(tr("&Find Symbol"), MENU_FIND_SYMBOL);
}

QtActions::Info QtActions::executeFindSymbol()
{
	return Info(tr("Search"), KEYBOARD_EXECUTE_SEARCH);
}

QtActions::Info QtActions::findText()
{
	return Info(tr("&Find Text"), MENU_FIND_TEXT);
}

QtActions::Info QtActions::nextReference()
{
	return Info(tr("Next Reference"), KEYBOARD_NEXT_REFERENCE);
}

QtActions::Info QtActions::previousReference()
{
	return Info(tr("Previous Reference"), KEYBOARD_PREVIOUS_REFERENCE);
}

QtActions::Info QtActions::nextLocalReference()
{
	return Info(tr("Next Local Reference"), KEYBOARD_NEXT_LOCAL_REFERENCE);
}

QtActions::Info QtActions::previousLocalReference()
{
	return Info(tr("Previous Local Reference"), KEYBOARD_PREVIOUS_LOCAL_REFERENCE);
}

QtActions::Info QtActions::customTrailDialog()
{
	return Info(tr("Custom Trail..."), KEYBOARD_CUSTOM_TRAIL_DIALOG);
}

QtActions::Info QtActions::toOverview()
{
	return Info(tr("&To overview"), KEYBOARD_TO_OVERVIEW);
}

QtActions::Info QtActions::saveGraphAsImage()
{
	return Info(tr("&Save Graph as Image..."), MENU_SAVE_AS);
}

QtActions::Info QtActions::saveAsImage()
{
	return Info(tr("Save as Image..."), MENU_SAVE_AS);
}

QtActions::Info QtActions::copyToClipboard()
{
	// Alternative: QKeySequence::Copy

	return Info(tr("Save to Clipboard"), KEYBOARD_SAVE_TO_CLIPBOARD);
}

QtActions::Info QtActions::preferences()
{
	// Alternative: QKeySequence::Preferences is only defined for MacOS!

	return Info(tr("Preferences..."), KEYBOARD_PREFERENCES);
}

QtActions::Info QtActions::newTab()
{
	return Info(tr("New Tab"), MENU_NEW_TAB);
}

QtActions::Info QtActions::closeTab()
{
	return Info(tr("Close Tab"), {
		MENU_CLOSE_TAB,
		KEYBOARD_CLOSE_TAB,
		MOUSE_CLOSE_TAB
	});
}

QtActions::Info QtActions::selectNextTab()
{
	// Alternative: 'QKeySequence::NextChild'

	return Info(tr("Select Next Tab"), KEYBOARD_SELECT_NEXT_TAB);
}

QtActions::Info QtActions::selectPreviousTab()
{
	// Alternative: 'QKeySequence::PreviousChild'

	return Info(tr("Select Previous Tab"), KEYBOARD_SELECT_PREVIOUS_TAB);
}

QtActions::Info QtActions::largerFont()
{
	return Info(tr("Larger Font"), MENU_LARGER_FONT);
}

QtActions::Info QtActions::smallerFont()
{
	return Info(tr("Smaller Font"), MENU_SMALLER_FONT);
}

QtActions::Info QtActions::resetFontSize()
{
	return Info(tr("Reset Font Size"), MENU_RESET_FONT);
}

QtActions::Info QtActions::switchGraphCodeFocus()
{
	return Info(tr("Switch Focus between Graph and Code"), KEYBOARD_SWITCH_GRAPH_CODE_FOCUS);
}

QtActions::Info QtActions::moveCodeFocus()
{
	return Info(tr("Move Focus Within Code"), {
		KEYBOARD_MOVE_FOCUS_LEFT,      KEYBOARD_MOVE_FOCUS_RIGHT,      KEYBOARD_MOVE_FOCUS_UP,      KEYBOARD_MOVE_FOCUS_DOWN,
		KEYBOARD_MOVE_VIM_FOCUS_LEFT,  KEYBOARD_MOVE_VIM_FOCUS_RIGHT,  KEYBOARD_MOVE_VIM_FOCUS_UP,  KEYBOARD_MOVE_VIM_FOCUS_DOWN,
		KEYBOARD_MOVE_GAME_FOCUS_LEFT, KEYBOARD_MOVE_GAME_FOCUS_RIGHT, KEYBOARD_MOVE_GAME_FOCUS_UP, KEYBOARD_MOVE_GAME_FOCUS_DOWN
	});
}

QtActions::Info QtActions::moveReferenceFocus()
{
	return Info(tr("Move Focus to Closest Reference"), {
		KEYBOARD_MOVE_REFERENCE_FOCUS_LEFT,      KEYBOARD_MOVE_REFERENCE_FOCUS_RIGHT,      KEYBOARD_MOVE_REFERENCE_FOCUS_UP,      KEYBOARD_MOVE_REFERENCE_FOCUS_DOWN,
		KEYBOARD_MOVE_VIM_REFERENCE_FOCUS_LEFT,  KEYBOARD_MOVE_VIM_REFERENCE_FOCUS_RIGHT,  KEYBOARD_MOVE_VIM_REFERENCE_FOCUS_UP,  KEYBOARD_MOVE_VIM_REFERENCE_FOCUS_DOWN,
		KEYBOARD_MOVE_GAME_REFERENCE_FOCUS_LEFT, KEYBOARD_MOVE_GAME_REFERENCE_FOCUS_RIGHT, KEYBOARD_MOVE_GAME_REFERENCE_FOCUS_UP, KEYBOARD_MOVE_GAME_REFERENCE_FOCUS_DOWN,
	});
}

QtActions::Info QtActions::activateLocation()
{
	return Info(tr("Activate Location"), {
		KEYBOARD_ACTIVATE_FOCUS_1,
		KEYBOARD_ACTIVATE_FOCUS_2
	});
}

QtActions::Info QtActions::activateLocationNewTab()
{
	return Info(tr("Activate Location in New Tab"), {
		KEYBOARD_ACTIVATE_FOCUS_IN_NEW_TAB_1,
		KEYBOARD_ACTIVATE_FOCUS_IN_NEW_TAB_2
	});
}

QtActions::Info QtActions::scrollCodeArea()
{
	return Info(tr("Scroll Code Area"), {
		KEYBOARD_MOVE_VIEW_LEFT,
		KEYBOARD_MOVE_VIEW_RIGHT,
		KEYBOARD_MOVE_VIEW_UP,
		KEYBOARD_MOVE_VIEW_DOWN
	});
}

QtActions::Info QtActions::moveNodeFocus()
{
	return Info(tr("Move Focus Within Nodes"), {
		KEYBOARD_MOVE_FOCUS_LEFT,      KEYBOARD_MOVE_FOCUS_RIGHT,      KEYBOARD_MOVE_FOCUS_UP,      KEYBOARD_MOVE_FOCUS_DOWN,
		KEYBOARD_MOVE_VIM_FOCUS_LEFT,  KEYBOARD_MOVE_VIM_FOCUS_RIGHT,  KEYBOARD_MOVE_VIM_FOCUS_UP,  KEYBOARD_MOVE_VIM_FOCUS_DOWN,
		KEYBOARD_MOVE_GAME_FOCUS_LEFT, KEYBOARD_MOVE_GAME_FOCUS_RIGHT, KEYBOARD_MOVE_GAME_FOCUS_UP, KEYBOARD_MOVE_GAME_FOCUS_DOWN
	});
}

QtActions::Info QtActions::moveEdgeFocus()
{
	return Info(tr("Move Focus Within Edges"), {
		KEYBOARD_MOVE_REFERENCE_FOCUS_LEFT,      KEYBOARD_MOVE_REFERENCE_FOCUS_RIGHT,      KEYBOARD_MOVE_REFERENCE_FOCUS_UP,      KEYBOARD_MOVE_REFERENCE_FOCUS_DOWN,
		KEYBOARD_MOVE_VIM_REFERENCE_FOCUS_LEFT,  KEYBOARD_MOVE_VIM_REFERENCE_FOCUS_RIGHT,  KEYBOARD_MOVE_VIM_REFERENCE_FOCUS_UP,  KEYBOARD_MOVE_VIM_REFERENCE_FOCUS_DOWN,
		KEYBOARD_MOVE_GAME_REFERENCE_FOCUS_LEFT, KEYBOARD_MOVE_GAME_REFERENCE_FOCUS_RIGHT, KEYBOARD_MOVE_GAME_REFERENCE_FOCUS_UP, KEYBOARD_MOVE_GAME_REFERENCE_FOCUS_DOWN,
	});
}

QtActions::Info QtActions::activateNodeEdge()
{
	return Info(tr("Activate Node/Edge"), {
		KEYBOARD_ACTIVATE_FOCUS_1,
		KEYBOARD_ACTIVATE_FOCUS_2
	});
}

QtActions::Info QtActions::expandOrCollapseNode()
{
	return Info(tr("Expand/Collapse Node"), {
		KEYBOARD_EXPAND_FOCUS_1,
		KEYBOARD_EXPAND_FOCUS_2
	});

}

QtActions::Info QtActions::activateNodeNewTab()
{
	return Info(tr("Activate Node in New Tab"), {
		KEYBOARD_ACTIVATE_FOCUS_IN_NEW_TAB_1,
		KEYBOARD_ACTIVATE_FOCUS_IN_NEW_TAB_2
	});
}

QtActions::Info QtActions::scrollGraphArea()
{
	return Info(tr("Scroll Graph Area"), {
		KEYBOARD_MOVE_VIEW_LEFT,
		KEYBOARD_MOVE_VIEW_RIGHT,
		KEYBOARD_MOVE_VIEW_UP,
		KEYBOARD_MOVE_VIEW_DOWN
	});
}

QtActions::Info QtActions::refreshUI()
{
	return Info(tr("Refresh UI"), KEYBOARD_REFRESH_UI);
}

QtActions::Info QtActions::zoomIn()
{
	return Info(tr("Zoom in"), {
		KEYBOARD_ZOOM_IN,
		MOUSE_ZOOM_IN
	});
}

QtActions::Info QtActions::zoomInWithMouse()
{
	return Info(tr("Zoom in"), MOUSE_ZOOM_IN);
}

QtActions::Info QtActions::zoomOut()
{
	return Info(tr("Zoom out"), {
		KEYBOARD_ZOOM_OUT,
		MOUSE_ZOOM_OUT
	});
}

QtActions::Info QtActions::zoomOutWithMouse()
{
	return Info(tr("Zoom out"), MOUSE_ZOOM_OUT);
}

QtActions::Info QtActions::resetZoom()
{
	return Info(tr("Reset Zoom"), KEYBOARD_RESET_ZOOM);
}


QtActions::Info QtActions::expandNodeWithMouse()
{
	return Info(tr("Expand Node"), MOUSE_EXPAND);
}

QtActions::Info QtActions::collapseNodeWithMouse()
{
	return Info(tr("Collapse Node"), MOUSE_COLLAPSE);
}

QtActions::Info QtActions::back()
{
	return Info(tr("Back"), {
		MENU_HISTORY_UNDO,
		KEYBOARD_HISTORY_UNDO_1,
		KEYBOARD_HISTORY_UNDO_2,
		KEYBOARD_HISTORY_UNDO_3,
	});
}

QtActions::Info QtActions::forward()
{
	return Info(tr("Forward"), {
		MENU_HISTORY_REDO,
		KEYBOARD_HISTORY_REDO_1,
		KEYBOARD_HISTORY_REDO_2
	});
}

QtActions::Info QtActions::bookmarkActiveSymbol()
{
	return Info(tr("Bookmark Active Symbol..."), KEYBOARD_BOOKMARK);
}

QtActions::Info QtActions::bookmarkNode()
{
	return Info(tr("Bookmark Node..."), KEYBOARD_BOOKMARK);
}

QtActions::Info QtActions::bookmarkManager()
{
	return Info(tr("Bookmark Manager..."), KEYBOARD_BOOKMARK_MANAGER);
}

QtActions::Info QtActions::activateBookmark()
{
	return Info(tr("Activate bookmark"), {
		MOUSE_ACTIVATE_BOOKMARK_1,
		MOUSE_ACTIVATE_BOOKMARK_2
	});
}

QtActions::Info QtActions::findOnScreen()
{
	return Info(tr("&Find On-Screen"), {
		KEYBOARD_SCREEN_SEARCH_1,

		// Handled in QtMainWindow::keyPressEvent:
		KEYBOARD_SCREEN_SEARCH_2,
		KEYBOARD_SCREEN_SEARCH_3
	});
}

QtActions::Info QtActions::screenSearchNext()
{
	return Info(tr("Next match"), KEYBOARD_NEXT_MATCH);
}

QtActions::Info QtActions::screenSearchPrevious()
{
	return Info(tr("Previous match"), KEYBOARD_PREVIOUS_MATCH);
}

QtActions::Info QtActions::screenSearchClose()
{
	return Info(tr("Close"), KEYBOARD_CANCEL);
}

// 'Help' keys:

QtActions::Info QtActions::showKeyboardShortcuts()
{
	return Info(tr("Keyboard Shortcuts..."), KEYBOARD_SHOW_KEYBOARD_SHORTCUTS);
}

QtActions::Info QtActions::showLegend()
{
	return Info(tr("Show legend"), KEYBOARD_SHOW_LEGEND);
}

// Miscellaneous:

QtActions::Info QtActions::textEncoding()
{
	return Info(tr("Text Encoding"), KEYBOARD_PREFERENCES);
}

// Pseudo mouse keys:

QtActions::Info QtActions::showInIDEWithMouse()
{
	return Info(tr("Show Definition in IDE"), MOUSE_SHOW_IN_IDE);
}

QtActions::Info QtActions::openInNewTabWithMouse()
{
	return Info(tr("Open in New Tab"), MOUSE_OPEN_IN_NEW_TAB);
}

QtActions::Info QtActions::showDefinitionWithMouse()
{
	return Info(tr("Show Definition"), MOUSE_SHOW_DEFINITION);
}

QtActions::Info QtActions::hideNodeWithMouse()
{
	return Info(tr("Hide Node"), MOUSE_HIDE);
}

QtActions::Info QtActions::hideEdgeWithMouse()
{
	return Info(tr("Hide Edge"), MOUSE_HIDE);
}


