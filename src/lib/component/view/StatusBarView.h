#ifndef STATUS_BAR_VIEW_H
#define STATUS_BAR_VIEW_H

#include "ErrorCountInfo.h"
#include "View.h"

class StatusBarController;

class StatusBarView: public View
{
public:
	StatusBarView(ViewLayout* viewLayout);
	~StatusBarView() override = default;

	std::string getName() const override;
	virtual void showMessage(const std::string& message, bool isError, bool showLoader) = 0;
	virtual void setErrorCount(ErrorCountInfo errorCount) = 0;

	virtual void showIdeStatus(const std::string& message) = 0;

	virtual void showIndexingProgress(size_t progressPercent) = 0;
	virtual void hideIndexingProgress() = 0;

protected:
	StatusBarController* getController();
};

#endif	  // STATUS_BAR_VIEW_H
