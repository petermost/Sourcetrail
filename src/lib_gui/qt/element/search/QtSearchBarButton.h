#ifndef QT_SEARCH_BAR_BUTTON_H
#define QT_SEARCH_BAR_BUTTON_H

#include "QtSelfRefreshIconButton.h"

class QtSearchBarButton: public QtSelfRefreshIconButton
{
public:
	enum class Size
	{
		SMALL,
		BIG
	};

	QtSearchBarButton(const FilePath& iconPath, Size size = Size::BIG);

protected:
	void refresh() override;

private:
	Size m_size;
};

#endif	  // QT_SEARCH_BAR_BUTTON_H
