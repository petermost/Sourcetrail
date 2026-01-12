#include "utilityQt.h"

#include "FilePath.h"
#include "FileSystem.h"
#include "QtMainView.h"
#include "logging.h"
#include "ColorScheme.h"

#include <QDir>
#include <QFile>
#include <QFontDatabase>
#include <QIcon>
#include <QPainter>
#include <QWidget>

#include <set>

namespace utility
{
void setWidgetBackgroundColor(QWidget *widget, const std::string &color)
{
	QPalette palette = widget->palette();
	palette.setColor(widget->backgroundRole(), QColor(color.c_str()));
	widget->setPalette(palette);
	widget->setAutoFillBackground(true);
}

void setWidgetRetainsSpaceWhenHidden(QWidget *widget)
{
	QSizePolicy pol = widget->sizePolicy();
	pol.setRetainSizeWhenHidden(true);
	widget->setSizePolicy(pol);
}

void loadFontsFromDirectory(const FilePath &path, const std::string &extension)
{
	std::vector<std::string> extensions;
	extensions.push_back(extension);
	std::vector<FilePath> fontFilePaths = FileSystem::getFilePathsFromDirectory(path, extensions);

	std::set<int> loadedFontIds;

	for (const FilePath &fontFilePath : fontFilePaths)
	{
		QFile file(QString::fromStdString(fontFilePath.str()));
		if (file.open(QIODevice::ReadOnly))
		{
			int id = QFontDatabase::addApplicationFontFromData(file.readAll());
			if (id != -1)
			{
				loadedFontIds.insert(id);
			}
		}
	}

	for (int loadedFontId : loadedFontIds)
	{
		for (QString &family : QFontDatabase::applicationFontFamilies(loadedFontId))
		{
			LOG_INFO("Loaded FontFamily: " + family.toStdString());
		}
	}
}

QPixmap colorizePixmap(const QPixmap &pixmap, QColor color)
{
	QImage image = pixmap.toImage();
	QImage colorImage(image.size(), image.format());
	QPainter colorPainter(&colorImage);
	colorPainter.fillRect(image.rect(), color);

	QPainter painter(&image);
	painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
	painter.drawImage(0, 0, colorImage);
	return QPixmap::fromImage(image);
}

QIcon createButtonIcon(const FilePath &iconPath, const std::string &colorId)
{
	ColorScheme *scheme = ColorScheme::getInstance().get();

	QPixmap pixmap(QString::fromStdString(iconPath.str()));
	QIcon icon(utility::colorizePixmap(pixmap, scheme->getColor(colorId + "/icon").c_str()));

	if (scheme->hasColor(colorId + "/icon_disabled"))
	{
		icon.addPixmap(utility::colorizePixmap(pixmap, scheme->getColor(colorId + "/icon_disabled").c_str()), QIcon::Disabled);
	}

	return icon;
}

QtMainWindow *getMainWindowforMainView(ViewLayout *viewLayout)
{
	if (QtMainView *mainView = dynamic_cast<QtMainView *>(viewLayout))
	{
		return mainView->getMainWindow();
	}
	return nullptr;
}

void copyNewFilesFromDirectory(const QString &src, const QString &dst)
{
	QDir dir(src);
	if (!dir.exists())
	{
		return;
	}

	for (const QString &d : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
	{
		QString dst_path = dst + QDir::separator() + d;

		dir.mkpath(dst_path);
		copyNewFilesFromDirectory(src + QDir::separator() + d, dst_path);
	}

	for (const QString &f : dir.entryList(QDir::Files))
	{
		if (!QFile::exists(dst + QDir::separator() + f))
		{
			QFile::copy(src + QDir::separator() + f, dst + QDir::separator() + f);
		}
	}
}

const char QT_AUTO_SCREEN_SCALE_FACTOR[] = "QT_AUTO_SCREEN_SCALE_FACTOR";

bool isQtAutoScreenScaleFactorEnabled()
{
	return qgetenv(QT_AUTO_SCREEN_SCALE_FACTOR) == "1";
}

void setQtAutoScreenScaleFactorEnabled(bool enable)
{
	QByteArray value;
	value.setNum(enable ? 1 : 0);

	qputenv(QT_AUTO_SCREEN_SCALE_FACTOR, value);
}

const char QT_SCALE_FACTOR[] = "QT_SCALE_FACTOR";

std::optional<double> getQtScaleFactor()
{
	bool ok;
	double scaleFactor = qgetenv(QT_SCALE_FACTOR).toDouble(&ok);
	if (ok)
		return scaleFactor;
	else
		return {};
}

void setQtScaleFactor(double factor)
{
	QByteArray value;
	value.setNum(factor);

	qputenv(QT_SCALE_FACTOR, value);
}

}
