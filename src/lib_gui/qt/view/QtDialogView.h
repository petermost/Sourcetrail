#ifndef QT_DIALOG_VIEW_H
#define QT_DIALOG_VIEW_H

#include "DialogView.h"

#include "QtThreadedFunctor.h"
#include "QtWindowStack.h"

#include "MessageErrorCountUpdate.h"
#include "MessageIndexingShowDialog.h"
#include "MessageListener.h"
#include "MessageWindowClosed.h"

class QtMainWindow;
class QtWindow;

class QtDialogView
	: public QObject
	, public DialogView
	, public MessageListener<MessageErrorCountUpdate>
	, public MessageListener<MessageIndexingShowDialog>
	, public MessageListener<MessageWindowClosed>
{
	Q_OBJECT

public:
	QtDialogView(QtMainWindow* mainWindow, UseCase useCase, StorageAccess* storageAccess);
	~QtDialogView() override;

	bool dialogsHidden() const override;
	void clearDialogs() override;

	void showUnknownProgressDialog(const std::string& title, const std::string& message) override;
	void hideUnknownProgressDialog() override;

	void showProgressDialog(
		const std::string& title, const std::string& message, size_t progress) override;
	void hideProgressDialog() override;

	void startIndexingDialog(
		Project* project,
		const std::vector<RefreshMode>& enabledModes,
		const RefreshMode initialMode,
		bool enabledShallowOption,
		bool initialShallowState,
		std::function<void(const RefreshInfo& info)> onStartIndexing,
		std::function<void()> onCancelIndexing) override;
	void updateIndexingDialog(
		size_t startedFileCount,
		size_t finishedFileCount,
		size_t totalFileCount,
		const std::vector<FilePath>& sourcePaths) override;
	void updateCustomIndexingDialog(
		size_t startedFileCount,
		size_t finishedFileCount,
		size_t totalFileCount,
		const std::vector<FilePath>& sourcePaths) override;
	DatabasePolicy finishedIndexingDialog(
		size_t indexedFileCount,
		size_t totalIndexedFileCount,
		size_t completedFileCount,
		size_t totalFileCount,
		float time,
		ErrorCountInfo errorInfo,
		bool interrupted,
		bool shallow) override;

	int confirm(const std::string& message, const std::vector<std::string>& options) override;

	void setParentWindow(QtWindow* window);

private slots:
	void showUnknownProgress(const std::string& title, const std::string& message, bool stacked);
	void hideUnknownProgress();

	void setUIBlocked(bool blocked);
	void dialogVisibilityChanged(bool visible);

private:
	void handleMessage(MessageErrorCountUpdate* message) override;
	void handleMessage(MessageIndexingShowDialog* message) override;
	void handleMessage(MessageWindowClosed* message) override;

	void updateErrorCount(size_t errorCount, size_t fatalCount);

	template <typename DialogType, typename... ParamTypes>
	DialogType* createWindow(ParamTypes... params);

	QtMainWindow* m_mainWindow;
	QtWindow* m_parentWindow = nullptr;

	QtWindowStack m_windowStack;

	QtThreadedLambdaFunctor m_onQtThread;
	QtThreadedLambdaFunctor m_onQtThread2;
	QtThreadedLambdaFunctor m_onQtThread3;

	std::map<RefreshMode, RefreshInfo> m_refreshInfos;
	bool m_shallowIndexingEnabled;

	std::atomic<bool> m_resultReady = false;
	bool m_uiBlocked = false;
	bool m_dialogsVisible = true;
};

#endif	  // QT_DIALOG_VIEW_H
