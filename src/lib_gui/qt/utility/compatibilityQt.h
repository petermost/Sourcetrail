#ifndef COMPATIBILITY_QT_H
#define COMPATIBILITY_QT_H

#include <QWheelEvent>

namespace utility::compatibility
{

<<<<<<< HEAD
QPoint QWheelEvent_globalPos(const QWheelEvent *event);
QPoint QMouseEvent_globalPos(const QMouseEvent *event);
int QMouseEvent_x(const QMouseEvent *event);

}
=======
QPoint QMouseEvent_globalPos(const QMouseEvent* event);

int QMouseEvent_x(const QMouseEvent* event);
}	 // namespace compatibility
}	 // namespace utility
>>>>>>> 09ccbe42a1120f7185e91e13d9d2b8583217be7f

#endif	  // COMPATIBILITY_QT_H
