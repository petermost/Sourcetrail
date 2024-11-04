#ifndef QT_FILE_DIALOG_H
#define QT_FILE_DIALOG_H

<<<<<<< HEAD
#include <QtContainerFwd>
=======
#include <QStringList>
>>>>>>> 09ccbe42a1120f7185e91e13d9d2b8583217be7f

class FilePath;
class QString;
class QWidget;

class QtFileDialog
{
public:
	static QStringList getFileNamesAndDirectories(QWidget* parent, const FilePath& path);

	static QString getExistingDirectory(QWidget* parent, const QString& caption, const FilePath& dir);
	static QString getOpenFileName(
		QWidget* parent, const QString& caption, const FilePath& dir, const QString& filter);

	static QString showSaveFileDialog(
		QWidget* parent, const QString& title, const FilePath& directory, const QString& filter);

private:
	static QString getDir(QString dir);
	static void saveFilePickerLocation(const FilePath& path);
};

#endif	  // QT_FILE_DIALOG_H
