#include "StatusBarController.h"

#include "StatusBarView.h"
#include "StorageAccess.h"
#include "logging.h"

StatusBarController::StatusBarController(StorageAccess* storageAccess)
	: m_storageAccess(storageAccess)
{
}

StatusBarController::~StatusBarController() = default;

StatusBarView* StatusBarController::getView()
{
	return Controller::getView<StatusBarView>();
}

void StatusBarController::clear()
{
	getView()->setErrorCount(ErrorCountInfo());
}

void StatusBarController::handleMessage(MessageErrorCountClear*  /*message*/)
{
	getView()->setErrorCount(ErrorCountInfo());
}

void StatusBarController::handleMessage(MessageErrorCountUpdate* message)
{
	getView()->setErrorCount(message->errorCount);
}

void StatusBarController::handleMessage(MessageIndexingFinished*  /*message*/)
{
	getView()->setErrorCount(m_storageAccess->getErrorCount());
	getView()->hideIndexingProgress();
}

void StatusBarController::handleMessage(MessageIndexingStarted*  /*message*/)
{
	getView()->showIndexingProgress(0);
}

void StatusBarController::handleMessage(MessageIndexingStatus* message)
{
	if (message->showProgress)
	{
		getView()->showIndexingProgress(message->progressPercent);
	}
	else
	{
		getView()->hideIndexingProgress();
	}
}

void StatusBarController::handleMessage(MessagePingReceived* message)
{
	std::string status = "No IDE connected";

	if (!message->ideName.empty())
	{
		status = "Connected to ";
		status += message->ideName;
	}

	getView()->showIdeStatus(status);
}

void StatusBarController::handleMessage(MessageRefresh*  /*message*/)
{
	getView()->setErrorCount(m_storageAccess->getErrorCount());
}

void StatusBarController::handleMessage(MessageStatus* message)
{
	if (message->showInStatusBar)
	{
		setStatus(message->status(), message->isError, message->showLoader);
	}
}

void StatusBarController::handleMessage(MessageTextEncodingChanged *message)
{
	getView()->showTextEncoding(message->textEncoding);
}

void StatusBarController::setStatus(const std::string& status, bool isError, bool showLoader)
{
	if (!status.empty())
	{
		LOG_INFO("STATUS " + status);
	}

	getView()->showMessage(status, isError, showLoader);
}
