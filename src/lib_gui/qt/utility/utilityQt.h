#ifndef UTILITY_QT_H
#define UTILITY_QT_H

#include <string>

#include <QVariant>

class FilePath;
class QColor;
class QIcon;
class QPixmap;
class QString;
class QWidget;
class QtMainWindow;
class ViewLayout;

namespace utility
{
void setWidgetBackgroundColor(QWidget *widget, const std::string &color);
void setWidgetRetainsSpaceWhenHidden(QWidget *widget);

void loadFontsFromDirectory(const FilePath &path, const std::string &extension = ".otf");

QPixmap colorizePixmap(const QPixmap &pixmap, QColor color);
QIcon createButtonIcon(const FilePath &iconPath, const std::string &colorId);

QtMainWindow *getMainWindowforMainView(ViewLayout *viewLayout);

void copyNewFilesFromDirectory(const QString &src, const QString &dst);

extern const char QT_AUTO_SCREEN_SCALE_FACTOR[];

bool isQtAutoScreenScaleFactorEnabled();
void setQtAutoScreenScaleFactorEnabled(bool enable);

extern const char QT_SCALE_FACTOR[];

std::optional<double> getQtScaleFactor();
void setQtScaleFactor(double factor);



template <typename T>
T qt_variant_cast(const QVariant &variant)
{
	Q_ASSERT(variant.canConvert<T>());
	return qvariant_cast<T>(variant);
}

}

#endif // UTILITY_QT_H
