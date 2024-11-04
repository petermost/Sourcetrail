#include "compatibilityQt.h"

#include <QtGlobal>


namespace utility::compatibility
{
QPoint QWheelEvent_globalPos(const QWheelEvent *event)
{
	return event->globalPosition().toPoint();
}

<<<<<<< HEAD
QPoint QMouseEvent_globalPos(const QMouseEvent *event)
{
	return event->globalPosition().toPoint();
}

int QMouseEvent_x(const QMouseEvent *event)
{
	return qRound(event->position().x());
}

} // namespace utility::compatibility

=======
QPoint QMouseEvent_globalPos(const QMouseEvent* event)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
	return event->globalPosition().toPoint();
#else
	return event->globalPos();
#endif
}

int QMouseEvent_x(const QMouseEvent* event)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
	return qRound(event->position().x());
#else
	return event->x();
#endif
}
}	 // namespace compatibility
}	 // namespace utility
>>>>>>> 09ccbe42a1120f7185e91e13d9d2b8583217be7f
