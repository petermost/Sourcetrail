#include "QtMainWindow.h"

#include "Application.h"
#include "ApplicationSettings.h"
#include "Bookmark.h"
#include "CompositeView.h"
#include "FileSystem.h"
#include "MessageActivateBase.h"
#include "MessageActivateLegend.h"
#include "MessageActivateOverview.h"
#include "MessageBookmarkActivate.h"
#include "MessageBookmarkBrowse.h"
#include "MessageBookmarkCreate.h"
#include "MessageCloseProject.h"
#include "MessageCodeReference.h"
#include "MessageCustomTrailShow.h"
#include "MessageErrorsHelpMessage.h"
#include "MessageFind.h"
#include "MessageFocusView.h"
#include "MessageHistoryRedo.h"
#include "MessageHistoryUndo.h"
#include "MessageIndexingShowDialog.h"
#include "MessageLoadProject.h"
#include "MessageRefresh.h"
#include "MessageRefreshUI.h"
#include "MessageResetZoom.h"
#include "MessageSaveAsImage.h"
#include "MessageTabClose.h"
#include "MessageTabOpen.h"
#include "MessageTabSelect.h"
#include "MessageWindowClosed.h"
#include "MessageZoom.h"
#include "QtAbout.h"
#include "QtActions.h"
#include "QtContextMenu.h"
#include "QtFileDialog.h"
#include "QtKeyboardShortcuts.h"
#include "QtLicenseWindow.h"
#include "QtPreferencesWindow.h"
#include "QtProjectWizard.h"
#include "QtResources.h"
#include "QtStartScreen.h"
#include "QtViewWidgetWrapper.h"
#include "ResourcePaths.h"
#include "TabbedView.h"
#include "UserPaths.h"
#include "View.h"
#include "logging.h"
#include "tracing.h"
#include "utilityApp.h"
#include "utilityQt.h"
#include "utilityString.h"

#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QDockWidget>
#include <QMenuBar>
#include <QSettings>
#include <QTimer>
#include <QToolBar>
#include <QToolTip>


using namespace utility;

QtViewToggle::QtViewToggle(View* view, QWidget* parent): QWidget(parent), m_view(view) {}

void QtViewToggle::clear()
{
	m_view = nullptr;
}

void QtViewToggle::toggledByAction()
{
	if (m_view)
	{
		dynamic_cast<QtMainWindow*>(parent())->toggleView(m_view, true);
	}
}

void QtViewToggle::toggledByUI()
{
	if (m_view)
	{
		dynamic_cast<QtMainWindow*>(parent())->toggleView(m_view, false);
	}
}


MouseReleaseFilter::MouseReleaseFilter(QObject* parent): QObject(parent)
{
	m_backButton = ApplicationSettings::getInstance()->getControlsMouseBackButton();
	m_forwardButton = ApplicationSettings::getInstance()->getControlsMouseForwardButton();
}

bool MouseReleaseFilter::eventFilter(QObject* obj, QEvent* event)
{
	if (event->type() == QEvent::MouseButtonRelease)
	{
		QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);

		if (mouseEvent->button() == m_backButton)
		{
			MessageHistoryUndo().dispatch();
			return true;
		}
		else if (mouseEvent->button() == m_forwardButton)
		{
			MessageHistoryRedo().dispatch();
			return true;
		}
	}

	return QObject::eventFilter(obj, event);
}

QtMainWindow::QtMainWindow()
	: m_windowStack(this)
{
	setObjectName(QStringLiteral("QtMainWindow"));
	setCentralWidget(nullptr);
	setDockNestingEnabled(true);

	setWindowIcon(QIcon(QString::fromUtf8(QtResources::ICON_LOGO_1024_1024)));
	setWindowFlags(Qt::Widget);

	QApplication *app = dynamic_cast<QApplication *>(QCoreApplication::instance());
	app->installEventFilter(new MouseReleaseFilter(this));

	refreshStyle();

	if constexpr (!utility::Platform::isMac())
	{
		// can only be done once, because resetting the style on the QCoreApplication causes crash
		app->setStyleSheet(QtResources::loadStyleSheet(QtResources::MAIN_SCROLLBAR_CSS));
	}

	setupProjectMenu();
	setupEditMenu();
	setupViewMenu();
	setupHistoryMenu();
	setupBookmarksMenu();
	setupHelpMenu();

	// Need to call loadLayout here for right DockWidget size on Linux
	// Second call is in Application.cpp
	loadLayout();
}

QtMainWindow::~QtMainWindow()
{
	for (DockWidget& dockWidget: m_dockWidgets)
	{
		dockWidget.toggle->clear();
	}
}

void QtMainWindow::addView(View* view)
{
	const QString name = QString::fromStdString(view->getName());
	if (name == QLatin1String("Tabs"))
	{
		QToolBar* toolBar = new QToolBar();
		toolBar->setObjectName("Tool" + name);
		toolBar->setMovable(false);
		toolBar->setFloatable(false);
		toolBar->setStyleSheet(QStringLiteral("* { margin: 0; }"));
		toolBar->addWidget(QtViewWidgetWrapper::getWidgetOfView(view));
		addToolBar(toolBar);
		return;
	}

	QDockWidget* dock = new QDockWidget(name, this);
	dock->setObjectName("Dock" + name);

	dock->setWidget(new QWidget());
	QVBoxLayout* layout = new QVBoxLayout(dock->widget());
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
	layout->addWidget(QtViewWidgetWrapper::getWidgetOfView(view));

	// Disable un-intended vertical growth of search widget
	if (name == QLatin1String("Search"))
	{
		dock->setSizePolicy(dock->sizePolicy().horizontalPolicy(), QSizePolicy::Fixed);
	}

	if (!m_showDockWidgetTitleBars)
	{
		dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
		dock->setTitleBarWidget(new QWidget());
	}

	addDockWidget(Qt::TopDockWidgetArea, dock);

	QtViewToggle* toggle = new QtViewToggle(view, this);
	connect(dock, &QDockWidget::visibilityChanged, toggle, &QtViewToggle::toggledByUI);

	QAction* action = new QAction(name + " Window", this);
	action->setCheckable(true);
	connect(action, &QAction::triggered, toggle, &QtViewToggle::toggledByAction);
	m_viewMenu->insertAction(m_viewSeparator, action);

	DockWidget dockWidget;
	dockWidget.widget = dock;
	dockWidget.view = view;
	dockWidget.action = action;
	dockWidget.toggle = toggle;

	m_dockWidgets.push_back(dockWidget);
}

void QtMainWindow::overrideView(View* view)
{
	const QString name = QString::fromStdString(view->getName());
	if (name == QLatin1String("Tabs"))
	{
		return;
	}

	QDockWidget* dock = nullptr;
	for (const DockWidget& dockWidget: m_dockWidgets)
	{
		// For unknown reason, the widget window title begins with an '&' (accelerator?), but none of
		// the views are created with one, so it is not clear whether this is related to:
		// https://bugreports.qt.io/browse/QTBUG-86407 "Accelerators in QDockWidget titles are displayed incorrectly in some styles".
		//
		// This workaround ensures that the views are at least found, but if the user enables 'Show Title Bars'
		// then the '&' is still shown!
		QString title = dockWidget.widget->windowTitle();
		if (title == name || (title.startsWith('&') && title.endsWith(name)))
		{
			dock = dockWidget.widget;
			break;
		}
	}

	if (!dock)
	{
		LOG_ERROR_STREAM(<< "Couldn't find view to override: " << name.toStdString());
		return;
	}

	QWidget* oldWidget = dock->widget()->layout()->itemAt(0)->widget();
	QWidget* newWidget = QtViewWidgetWrapper::getWidgetOfView(view);

	if (oldWidget == newWidget)
	{
		return;
	}

	oldWidget = dock->widget()->layout()->takeAt(0)->widget();
	oldWidget->hide();
	dock->widget()->layout()->addWidget(newWidget);
	newWidget->show();
}

void QtMainWindow::removeView(View* view)
{
	for (size_t i = 0; i < m_dockWidgets.size(); i++)
	{
		if (m_dockWidgets[i].view == view)
		{
			removeDockWidget(m_dockWidgets[i].widget);
			m_dockWidgets.erase(m_dockWidgets.begin() + i);
			return;
		}
	}
}

void QtMainWindow::showView(View* view)
{
	getDockWidgetForView(view)->widget->setHidden(false);
}

void QtMainWindow::hideView(View* view)
{
	getDockWidgetForView(view)->widget->setHidden(true);
}

View* QtMainWindow::findFloatingView(const std::string& name) const
{
	for (size_t i = 0; i < m_dockWidgets.size(); i++)
	{
		if (std::string(m_dockWidgets[i].view->getName()) == name &&
			m_dockWidgets[i].widget->isFloating())
		{
			return m_dockWidgets[i].view;
		}
	}

	return nullptr;
}

void QtMainWindow::loadLayout()
{
	QSettings settings(
		QString::fromStdString(UserPaths::getWindowSettingsFilePath().str()), QSettings::IniFormat);

	settings.beginGroup(QStringLiteral("MainWindow"));
	resize(settings.value(QStringLiteral("size"), QSize(600, 400)).toSize());
	move(settings.value(QStringLiteral("position"), QPoint(200, 200)).toPoint());
	if (settings.value(QStringLiteral("maximized"), false).toBool())
	{
		showMaximized();
	}
	setShowDockWidgetTitleBars(settings.value(QStringLiteral("showTitleBars"), true).toBool());
	settings.endGroup();
	loadDockWidgetLayout();
}

void QtMainWindow::loadDockWidgetLayout()
{
	QSettings settings(
		QString::fromStdString(UserPaths::getWindowSettingsFilePath().str()), QSettings::IniFormat);
	this->restoreState(settings.value(QStringLiteral("DOCK_LOCATIONS")).toByteArray());

	for (DockWidget dock: m_dockWidgets)
	{
		dock.action->setChecked(!dock.widget->isHidden());
	}
}

void QtMainWindow::loadWindow(bool showStartWindow)
{
	if (showStartWindow)
	{
		showStartScreen();
	}
}

void QtMainWindow::saveLayout()
{
	QSettings settings(
		QString::fromStdString(UserPaths::getWindowSettingsFilePath().str()), QSettings::IniFormat);

	settings.beginGroup(QStringLiteral("MainWindow"));
	settings.setValue(QStringLiteral("maximized"), isMaximized());
	if (!isMaximized())
	{
		settings.setValue(QStringLiteral("size"), size());
		settings.setValue(QStringLiteral("position"), pos());
	}
	settings.setValue(QStringLiteral("showTitleBars"), m_showDockWidgetTitleBars);
	settings.endGroup();

	settings.setValue(QStringLiteral("DOCK_LOCATIONS"), this->saveState());
}

void QtMainWindow::updateHistoryMenu(std::shared_ptr<MessageBase> message)
{
	const size_t historyMenuSize = 20;

	if (message && dynamic_cast<MessageActivateBase*>(message.get()))
	{
		std::vector<SearchMatch> matches =
			dynamic_cast<MessageActivateBase*>(message.get())->getSearchMatches();
		if (matches.size() && !matches[0].text.empty())
		{
			std::vector<std::shared_ptr<MessageBase>> history = {message};
			std::set<SearchMatch> uniqueMatches = {matches[0]};

			for (std::shared_ptr<MessageBase> m: m_history)
			{
				if (uniqueMatches
						.insert(dynamic_cast<MessageActivateBase*>(m.get())->getSearchMatches()[0])
						.second)
				{
					history.push_back(m);

					if (history.size() >= historyMenuSize)
					{
						break;
					}
				}
			}

			m_history = history;
		}
	}

	setupHistoryMenu();
}

void QtMainWindow::clearHistoryMenu()
{
	m_history.clear();
	setupHistoryMenu();
}

void QtMainWindow::updateBookmarksMenu(const std::vector<std::shared_ptr<Bookmark>>& bookmarks)
{
	m_bookmarks = bookmarks;
	setupBookmarksMenu();
}

void QtMainWindow::setContentEnabled(bool enabled)
{
	for (QAction* action : menuBar()->actions())
	{
		action->setEnabled(enabled);
	}

	for (DockWidget& dock: m_dockWidgets)
	{
		dock.widget->setEnabled(enabled);
	}
}

void QtMainWindow::refreshStyle()
{
	setStyleSheet(QtResources::loadStyleSheet(QtResources::MAIN_CSS));

	QFont tooltipFont = QToolTip::font();
	tooltipFont.setPixelSize(ApplicationSettings::getInstance()->getFontSize());
	QToolTip::setFont(tooltipFont);
}

void QtMainWindow::setWindowTitleProgress(size_t fileCount, size_t totalFileCount)
{
	m_windowTitleProgress.setProgress(fileCount, totalFileCount);
}

void QtMainWindow::hideWindowTitleProgress()
{
	m_windowTitleProgress.hideProgress();
}

void QtMainWindow::alert()
{
	QApplication::alert(this);
}

void QtMainWindow::showEvent(QShowEvent*  /*e*/)
{
	m_windowTitleProgress.setWindow(this);
}

void QtMainWindow::keyPressEvent(QKeyEvent* event)
{
	switch (event->key())
	{
	case Qt::Key_Backspace:
		MessageHistoryUndo().dispatch();
		break;

	case Qt::Key_Escape:
		emit hideScreenSearch();
		emit hideIndexingDialog();
		break;

	case Qt::Key_Slash:
	case Qt::Key_Question:
		emit showScreenSearch();
		break;

	case Qt::Key_R:
		if (event->modifiers() & (Qt::ControlModifier | Qt::AltModifier))
		{
			MessageRefreshUI().dispatch();
		}
		break;

	case Qt::Key_F4:
		if (event->modifiers() & Qt::ControlModifier)
		{
			closeTab();
		}
		break;

	case Qt::Key_Space:
		PRINT_TRACES();
		break;

	case Qt::Key_Tab:
		MessageFocusView(MessageFocusView::ViewType::TOGGLE).dispatch();
		break;
	}
}

void QtMainWindow::contextMenuEvent(QContextMenuEvent* event)
{
	QtContextMenu menu(event, this);
	menu.addUndoActions();
	menu.show();
}

void QtMainWindow::closeEvent(QCloseEvent*  /*event*/)
{
	MessageWindowClosed().dispatchImmediately();
}

void QtMainWindow::resizeEvent(QResizeEvent* event)
{
	m_windowStack.centerSubWindows();
	QMainWindow::resizeEvent(event);
}

bool QtMainWindow::focusNextPrevChild(bool  /*next*/)
{
	// makes tab key available in key press event
	return false;
}

void QtMainWindow::about()
{
	QtAbout* aboutWindow = createWindow<QtAbout>();
	aboutWindow->setupAbout();
}

void QtMainWindow::openSettings()
{
	QtPreferencesWindow* window = createWindow<QtPreferencesWindow>();
	window->setup();
}

void QtMainWindow::showDocumentation()
{
	QDesktopServices::openUrl(QUrl(QString::fromStdString(utility::getDocumentationLink())));
}

void QtMainWindow::showKeyboardShortcuts()
{
	QtKeyboardShortcuts* keyboardShortcutWindow = createWindow<QtKeyboardShortcuts>();
	keyboardShortcutWindow->setup();
}

void QtMainWindow::showLegend()
{
	MessageActivateLegend().dispatch();
}

void QtMainWindow::showErrorHelpMessage()
{
	MessageErrorsHelpMessage(true).dispatch();
}

void QtMainWindow::showChangelog()
{
	QDesktopServices::openUrl(QUrl(QStringLiteral("https://github.com/petermost/Sourcetrail/blob/master/CHANGELOG.md")));
}

void QtMainWindow::showBugtracker()
{
	QDesktopServices::openUrl(
		QUrl(QStringLiteral("https://github.com/petermost/Sourcetrail/issues")));
}

void QtMainWindow::showLicenses()
{
	QtLicenseWindow* licenseWindow = createWindow<QtLicenseWindow>();
	licenseWindow->setup();
}

void QtMainWindow::showDataFolder()
{
	QDesktopServices::openUrl(QUrl(
		QString::fromStdString(
			"file:///" + UserPaths::getUserDataDirectoryPath().makeCanonical().str()),
		QUrl::TolerantMode));
}

void QtMainWindow::showLogFolder()
{
	QDesktopServices::openUrl(QUrl(
		QString::fromStdString(
			"file:///" + ApplicationSettings::getInstance()->getLogDirectoryPath().str()),
		QUrl::TolerantMode));
}

void QtMainWindow::openTab()
{
	MessageTabOpen().dispatch();
}

void QtMainWindow::closeTab()
{
	MessageTabClose().dispatch();
}

void QtMainWindow::nextTab()
{
	MessageTabSelect(true).dispatch();
}

void QtMainWindow::previousTab()
{
	MessageTabSelect(false).dispatch();
}

void QtMainWindow::showStartScreen()
{
	if (dynamic_cast<QtStartScreen*>(m_windowStack.getTopWindow()))
	{
		return;
	}

	QtStartScreen* startScreen = createWindow<QtStartScreen>();
	connect(startScreen, &QtStartScreen::openOpenProjectDialog, this, &QtMainWindow::openProject);
	connect(startScreen, &QtStartScreen::openNewProjectDialog, this, &QtMainWindow::newProject);
}

void QtMainWindow::hideStartScreen()
{
	m_windowStack.clearWindows();
}

void QtMainWindow::newProject()
{
	QtProjectWizard* wizard = createWindow<QtProjectWizard>();
	wizard->newProject();
}

void QtMainWindow::newProjectFromCDB(const FilePath& filePath)
{
	QtProjectWizard* wizard = dynamic_cast<QtProjectWizard*>(m_windowStack.getTopWindow());
	if (!wizard)
	{
		wizard = createWindow<QtProjectWizard>();
	}

	wizard->newProjectFromCDB(filePath);
}

void QtMainWindow::openProject()
{
	QString fileName = QtFileDialog::getOpenFileName(
		this, tr("Open File"), FilePath(), QStringLiteral("Sourcetrail Project Files (*.srctrlprj)"));

	if (!fileName.isEmpty())
	{
		MessageLoadProject(FilePath(fileName.toStdString())).dispatch();
		m_windowStack.clearWindows();
	}
}

void QtMainWindow::editProject()
{
	std::shared_ptr<const Project> currentProject = Application::getInstance()->getCurrentProject();
	if (currentProject)
	{
		QtProjectWizard* wizard = createWindow<QtProjectWizard>();

		wizard->editProject(currentProject->getProjectSettingsFilePath());
	}
}

void QtMainWindow::closeProject()
{
	if (Application::getInstance()->getCurrentProject())
	{
		MessageCloseProject().dispatch();
		showStartScreen();
	}
}

void QtMainWindow::find()
{
	MessageFind().dispatch();
}

void QtMainWindow::findFulltext()
{
	MessageFind(true).dispatch();
}

void QtMainWindow::findOnScreen()
{
	emit showScreenSearch();
}

void QtMainWindow::codeReferencePrevious()
{
	MessageCodeReference(MessageCodeReference::REFERENCE_PREVIOUS, false).dispatch();
}

void QtMainWindow::codeReferenceNext()
{
	MessageCodeReference(MessageCodeReference::REFERENCE_NEXT, false).dispatch();
}

void QtMainWindow::codeLocalReferencePrevious()
{
	MessageCodeReference(MessageCodeReference::REFERENCE_PREVIOUS, true).dispatch();
}

void QtMainWindow::codeLocalReferenceNext()
{
	MessageCodeReference(MessageCodeReference::REFERENCE_NEXT, true).dispatch();
}

void QtMainWindow::customTrail()
{
	MessageCustomTrailShow().dispatch();
}

void QtMainWindow::overview()
{
	MessageActivateOverview().dispatch();
}

void QtMainWindow::closeWindow()
{
	QWidget* activeWindow = QApplication::activeWindow();
	if (activeWindow)
	{
		activeWindow->close();
	}
}

void QtMainWindow::refresh()
{
	MessageIndexingShowDialog().dispatch();
	MessageRefresh().dispatch();
}

void QtMainWindow::forceRefresh()
{
	MessageIndexingShowDialog().dispatch();
	MessageRefresh().refreshAll().dispatch();
}

void QtMainWindow::saveAsImage()
{
	QString filePath = QtFileDialog::showSaveFileDialog(
		this, tr("Save as Image"), FilePath(), "PNG (*.png);;JPEG (*.JPEG);;BMP Files (*.bmp)");
	if (filePath.isNull())
	{
		return;
	}
	MessageSaveAsImage m(filePath);
	m.setSchedulerId(TabIds::currentTab());
	m.dispatch();
}

void QtMainWindow::undo()
{
	MessageHistoryUndo().dispatch();
}

void QtMainWindow::redo()
{
	MessageHistoryRedo().dispatch();
}

void QtMainWindow::zoomIn()
{
	MessageZoom(true).dispatch();
}

void QtMainWindow::zoomOut()
{
	MessageZoom(false).dispatch();
}

void QtMainWindow::resetZoom()
{
	MessageResetZoom().dispatch();
}

void QtMainWindow::resetWindowLayout()
{
	FileSystem::remove(UserPaths::getWindowSettingsFilePath());
	FileSystem::copyFile(
		ResourcePaths::getFallbackDirectoryPath().concatenate("window_settings.ini"),
		UserPaths::getWindowSettingsFilePath());
	loadDockWidgetLayout();
}

void QtMainWindow::openRecentProject()
{
	QAction* action = qobject_cast<QAction*>(sender());
	if (action)
	{
		MessageLoadProject(FilePath(action->data().toString().toStdString())).dispatch();
		m_windowStack.clearWindows();
	}
}

void QtMainWindow::updateRecentProjectsMenu()
{
	m_recentProjectsMenu->clear();

	const std::vector<FilePath> recentProjects =
		ApplicationSettings::getInstance()->getRecentProjects();
	const size_t recentProjectsCount = ApplicationSettings::getInstance()->getMaxRecentProjectsCount();

	for (size_t i = 0; i < recentProjects.size() && i < recentProjectsCount; ++i)
	{
		const FilePath& project = recentProjects[i];
		if (project.exists())
		{
			QAction* recentProject = new QAction(this);
			recentProject->setText(QString::fromStdString(project.fileName()));
			recentProject->setData(QString::fromStdString(project.str()));
			connect(recentProject, &QAction::triggered, this, &QtMainWindow::openRecentProject);
			m_recentProjectsMenu->addAction(recentProject);
		}
	}
}

void QtMainWindow::toggleView(View* view, bool fromMenu)
{
	DockWidget* dock = getDockWidgetForView(view);

	if (fromMenu)
	{
		dock->widget->setVisible(dock->action->isChecked());
	}
	else
	{
		dock->action->setChecked(dock->widget->isVisible());
	}
}

void QtMainWindow::toggleShowDockWidgetTitleBars()
{
	setShowDockWidgetTitleBars(!m_showDockWidgetTitleBars);
}

void QtMainWindow::showBookmarkCreator()
{
	MessageBookmarkCreate().dispatch();
}

void QtMainWindow::showBookmarkBrowser()
{
	MessageBookmarkBrowse().dispatch();
}

void QtMainWindow::openHistoryAction()
{
	QAction* action = qobject_cast<QAction*>(sender());
	if (action)
	{
		std::shared_ptr<MessageBase> m = m_history[action->data().toInt()];
		m->setSchedulerId(TabIds::currentTab());
		m->setIsReplayed(false);
		m->dispatch();
	}
}

void QtMainWindow::activateBookmarkAction()
{
	QAction* action = qobject_cast<QAction*>(sender());
	if (action)
	{
		std::shared_ptr<Bookmark> bookmark = m_bookmarks[action->data().toInt()];
		MessageBookmarkActivate(bookmark).dispatch();
	}
}

void QtMainWindow::setupProjectMenu()
{
	QMenu* menu = new QMenu(tr("&Project"), this);
	menuBar()->addMenu(menu);

	menu->addAction(QtActions::newProject().text(), QtActions::newProject().shortcut(), this, &QtMainWindow::newProject);
	menu->addAction(QtActions::openProject().text(), QtActions::openProject().shortcut(), this, &QtMainWindow::openProject);

	m_recentProjectsMenu = new QMenu(tr("Recent Projects"));
	menu->addMenu(m_recentProjectsMenu);
	updateRecentProjectsMenu();

	menu->addSeparator();

	menu->addAction(tr("&Edit Project..."), this, &QtMainWindow::editProject);

	menu->addSeparator();

	menu->addAction(tr("Close Project"), this, &QtMainWindow::closeProject);
	menu->addAction(QtActions::exit().text(), QtActions::exit().shortcut(), QCoreApplication::instance(), &QCoreApplication::quit);
}

void QtMainWindow::setupEditMenu()
{
	QMenu* menu = new QMenu(tr("&Edit"), this);
	menuBar()->addMenu(menu);

	menu->addAction(QtActions::refresh().text(), QtActions::refresh().shortcut(), this, &QtMainWindow::refresh);
	menu->addAction(QtActions::fullRefresh().text(), QtActions::fullRefresh().shortcut(), this, &QtMainWindow::forceRefresh);

	menu->addSeparator();

	menu->addAction(QtActions::findSymbol().text(), QtActions::findSymbol().shortcut(), this, &QtMainWindow::find);
	menu->addAction(QtActions::findText().text(), QtActions::findText().shortcut(), this, &QtMainWindow::findFulltext);
	menu->addAction(QtActions::findOnScreen().text(), QtActions::findOnScreen().shortcut(), this, &QtMainWindow::findOnScreen);

	menu->addSeparator();

	menu->addAction(QtActions::nextReference().text(), QtActions::nextReference().shortcut(), this, &QtMainWindow::codeReferenceNext);
	menu->addAction(QtActions::previousReference().text(), QtActions::previousReference().shortcut(), this, &QtMainWindow::codeReferencePrevious);
	menu->addAction(QtActions::nextLocalReference().text(), QtActions::nextLocalReference().shortcut(), this, &QtMainWindow::codeLocalReferenceNext);
	menu->addAction(QtActions::previousLocalReference().text(), QtActions::previousLocalReference().shortcut(), this, &QtMainWindow::codeLocalReferencePrevious);

	menu->addSeparator();

	menu->addAction(QtActions::customTrailDialog().text(), QtActions::customTrailDialog().shortcut(), this, &QtMainWindow::customTrail);

	menu->addSeparator();

	menu->addAction(QtActions::toOverview().text(), QtActions::toOverview().shortcut(), this, &QtMainWindow::overview);

	menu->addSeparator();

	menu->addAction(QtActions::saveGraphAsImage().text(), QtActions::saveGraphAsImage().shortcut(), this, &QtMainWindow::saveAsImage);

	menu->addSeparator();

	menu->addAction(QtActions::preferences().text(), QtActions::preferences().shortcut(), this, &QtMainWindow::openSettings);
}

void QtMainWindow::setupViewMenu()
{
	QMenu* menu = new QMenu(tr("&View"), this);
	menuBar()->addMenu(menu);

	menu->addAction(QtActions::newTab().text(), QtActions::newTab().shortcut(), this, &QtMainWindow::openTab);
	menu->addAction(QtActions::closeTab().text(), QtActions::closeTab().shortcut(), this, &QtMainWindow::closeTab);
	menu->addAction(QtActions::selectNextTab().text(), QtActions::selectNextTab().shortcut(), this, &QtMainWindow::nextTab);
	menu->addAction(QtActions::selectPreviousTab().text(), QtActions::selectPreviousTab().shortcut(), this, &QtMainWindow::previousTab);

	menu->addSeparator();

	menu->addAction(tr("Show Start Window"), this, &QtMainWindow::showStartScreen);

	m_showTitleBarsAction = new QAction(tr("Show Title Bars"), this);
	m_showTitleBarsAction->setCheckable(true);
	m_showTitleBarsAction->setChecked(m_showDockWidgetTitleBars);
	connect(m_showTitleBarsAction, &QAction::triggered, this, &QtMainWindow::toggleShowDockWidgetTitleBars);
	menu->addAction(m_showTitleBarsAction);

	menu->addAction(tr("Reset Window Layout"), this, &QtMainWindow::resetWindowLayout);

	menu->addSeparator();

	m_viewSeparator = menu->addSeparator();

	menu->addAction(QtActions::largerFont().text(), QtActions::largerFont().shortcut(), this, &QtMainWindow::zoomIn);
	menu->addAction(QtActions::smallerFont().text(), QtActions::smallerFont().shortcut(), this, &QtMainWindow::zoomOut);
	menu->addAction(QtActions::resetFontSize().text(), QtActions::resetFontSize().shortcut(), this, &QtMainWindow::resetZoom);

	m_viewMenu = menu;
}

void QtMainWindow::setupHistoryMenu()
{
	if (!m_historyMenu)
	{
		m_historyMenu = new QMenu(tr("&History"), this);
		menuBar()->addMenu(m_historyMenu);
	}
	else
		m_historyMenu->clear();

	m_historyMenu->addAction(QtActions::back().text(), QtActions::back().shortcut(), this, &QtMainWindow::undo);
	m_historyMenu->addAction(QtActions::forward().text(), QtActions::forward().shortcut(), this, &QtMainWindow::redo);

	m_historyMenu->addSection(tr("Recent Symbols"));

	for (size_t i = 0; i < m_history.size(); i++)
	{
		MessageActivateBase* msg = dynamic_cast<MessageActivateBase*>(m_history[i].get());
		if (!msg)
		{
			continue;
		}

		const SearchMatch match = msg->getSearchMatches()[0];
		const std::string name = utility::elide(match.getFullName(), utility::ELIDE_RIGHT, 50);

		QAction* action = new QAction();
		action->setText(QString::fromStdString(name));
		action->setData(QVariant(int(i)));

		connect(action, &QAction::triggered, this, &QtMainWindow::openHistoryAction);
		m_historyMenu->addAction(action);
	}
}

void QtMainWindow::setupBookmarksMenu()
{
	if (!m_bookmarksMenu)
	{
		m_bookmarksMenu = new QMenu(tr("&Bookmarks"), this);
		menuBar()->addMenu(m_bookmarksMenu);
	}
	else
		m_bookmarksMenu->clear();

	m_bookmarksMenu->addAction(QtActions::bookmarkActiveSymbol().text(), QtActions::bookmarkActiveSymbol().shortcut(), this, &QtMainWindow::showBookmarkCreator);
	m_bookmarksMenu->addAction(QtActions::bookmarkManager().text(), QtActions::bookmarkManager().shortcut(), this, &QtMainWindow::showBookmarkBrowser);

	m_bookmarksMenu->addSection(tr("Recent Bookmarks"));

	for (size_t i = 0; i < m_bookmarks.size(); i++)
	{
		Bookmark* bookmark = m_bookmarks[i].get();
		std::string name = utility::elide(bookmark->getName(), utility::ELIDE_RIGHT, 50);

		QAction* action = new QAction();
		action->setText(QString::fromStdString(name));
		action->setData(QVariant(int(i)));

		connect(action, &QAction::triggered, this, &QtMainWindow::activateBookmarkAction);
		m_bookmarksMenu->addAction(action);
	}
}

void QtMainWindow::setupHelpMenu()
{
	QMenu* menu = new QMenu(tr("&Help"), this);
	menuBar()->addMenu(menu);

	menu->addAction(QtActions::showKeyboardShortcuts().text(), QtActions::showKeyboardShortcuts().shortcut(), this, &QtMainWindow::showKeyboardShortcuts);
	menu->addAction(QtActions::showLegend().text(), QtActions::showLegend().shortcut(), &QtMainWindow::showLegend);

	menu->addSeparator();

	menu->addAction(tr("Fixing Errors..."), this, &QtMainWindow::showErrorHelpMessage);
	menu->addAction(tr("Documentation..."), this, &QtMainWindow::showDocumentation);
	menu->addAction(tr("Changelog..."), this, &QtMainWindow::showChangelog);
	menu->addAction(tr("Bug Tracker..."), this, &QtMainWindow::showBugtracker);

	menu->addSeparator();
	
	menu->addAction(tr("About Sourcetrail..."), this, &QtMainWindow::about);
	menu->addAction(tr("About Qt..."), this, QApplication::aboutQt);
	menu->addAction(tr("License..."), this, &QtMainWindow::showLicenses);

	menu->addSeparator();

	menu->addAction(tr("Show Data Folder..."), this, &QtMainWindow::showDataFolder);
	menu->addAction(tr("Show Log Folder..."), this, &QtMainWindow::showLogFolder);
}

QtMainWindow::DockWidget* QtMainWindow::getDockWidgetForView(View* view)
{
	for (DockWidget& dock: m_dockWidgets)
	{
		if (dock.view == view)
		{
			return &dock;
		}

		const CompositeView* compositeView = dynamic_cast<const CompositeView*>(dock.view);
		if (compositeView)
		{
			for (const View* v: compositeView->getViews())
			{
				if (v == view)
				{
					return &dock;
				}
			}
		}

		const TabbedView* tabbedView = dynamic_cast<const TabbedView*>(dock.view);
		if (tabbedView)
		{
			for (const View* v: tabbedView->getViews())
			{
				if (v == view)
				{
					return &dock;
				}
			}
		}
	}

	LOG_ERROR("DockWidget was not found for view.");
	return nullptr;
}

void QtMainWindow::setShowDockWidgetTitleBars(bool showTitleBars)
{
	m_showDockWidgetTitleBars = showTitleBars;

	if (m_showTitleBarsAction)
	{
		m_showTitleBarsAction->setChecked(showTitleBars);
	}

	for (DockWidget& dock: m_dockWidgets)
	{
		if (showTitleBars)
		{
			dock.widget->setFeatures(
				QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable |
				QDockWidget::DockWidgetFloatable);
			dock.widget->setTitleBarWidget(nullptr);
		}
		else
		{
			dock.widget->setFeatures(QDockWidget::NoDockWidgetFeatures);
			dock.widget->setTitleBarWidget(new QWidget());
		}
	}
}

template <typename T>
T* QtMainWindow::createWindow()
{
	T* window = new T(this);

	connect(window, &QtWindow::canceled, &m_windowStack, &QtWindowStack::popWindow);
	connect(window, &QtWindow::finished, &m_windowStack, &QtWindowStack::clearWindows);

	m_windowStack.pushWindow(window);

	return window;
}
