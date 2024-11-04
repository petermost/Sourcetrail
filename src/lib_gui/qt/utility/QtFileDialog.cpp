#include "QtFileDialog.h"

#include <QFileDialog>
#include <QListView>
#include <QRegularExpression>
#include <QTreeView>
#include <QRegExp>

#include "ApplicationSettings.h"
#include "FilePath.h"
#include "QtFilesAndDirectoriesDialog.h"
#include "utilityApp.h"

using namespace std;

QStringList QtFileDialog::getFileNamesAndDirectories(QWidget* parent, const FilePath& path)
{
	const QString dir = getDir(
		QString::fromStdWString((path.isDirectory() ? path : path.getParentDirectory()).wstr()));

	unique_ptr<QFileDialog> dialog = (utility::Platform::isMac() ? make_unique<QFileDialog>(parent) : make_unique<QtFilesAndDirectoriesDialog>(parent));

	if (!dir.isEmpty())
	{
		dialog->setDirectory(dir);
	}

	QListView* l = dialog->findChild<QListView*>(QStringLiteral("listView"));
	if (l)
	{
		l->setSelectionMode(QAbstractItemView::ExtendedSelection);
	}
	QTreeView* t = dialog->findChild<QTreeView*>();
	if (t)
	{
		t->setSelectionMode(QAbstractItemView::ExtendedSelection);
	}

	QStringList list;
	if (dialog->exec())
	{
		list = dialog->selectedFiles();
	}

	saveFilePickerLocation(FilePath(dialog->directory().path().toStdString()));

	return list;
}

QString QtFileDialog::getExistingDirectory(QWidget* parent, const QString& caption, const FilePath& dir)
{
	const QString dirPath = QFileDialog::getExistingDirectory(
		parent, caption, getDir(QString::fromStdWString(dir.wstr())));
	saveFilePickerLocation(FilePath(dirPath.toStdString()));
	return dirPath;
}

QString QtFileDialog::getOpenFileName(
	QWidget* parent, const QString& caption, const FilePath& dir, const QString& filter)
{
	const QString filePath = QFileDialog::getOpenFileName(
		parent, caption, getDir(QString::fromStdWString(dir.wstr())), filter);
	const FilePath dirPath = FilePath(filePath.toStdString()).getParentDirectory();
	saveFilePickerLocation(dirPath);
	return filePath;
}

QString QtFileDialog::showSaveFileDialog(
	QWidget* parent, const QString& title, const FilePath& directory, const QString& filter)
{
	// Workaround for: "QFileDialog::getSaveFileName() does not append the file extension of the selected filter (QWidget-based, non-native)"
	// https://bugreports.qt.io/browse/QTBUG-27186

	QFileDialog dialog(parent, title, getDir(QString::fromStdWString(directory.wstr())), filter);
	dialog.setWindowModality(Qt::WindowModal);
	dialog.setAcceptMode(QFileDialog::AcceptSave);

	if (dialog.exec() == QDialog::Accepted) {
		QString selectedFileName = dialog.selectedFiles().constFirst();
		QFileInfo fileInfo(selectedFileName);

		if (fileInfo.suffix().isEmpty() && !dialog.selectedNameFilter().isEmpty()) {
			QRegularExpression filterRegEx(QStringLiteral("(?:^\\*\\.(?!.*\\()|\\(\\*\\.)(\\w+)"));
			QRegularExpressionMatch filterMatch = filterRegEx.match(dialog.selectedNameFilter());
			if (filterMatch.hasMatch()) {
				QString extension = filterMatch.captured(1);
				selectedFileName += QStringLiteral(".") + extension;
			}
		}
		return selectedFileName;
	} else
		return QString();
}

QString QtFileDialog::getDir(QString dir)
{
	if (!dir.isEmpty())
	{
		return dir;
	}

	dir = QString::fromStdString(
		ApplicationSettings::getInstance()->getLastFilepickerLocation().str());

	if (dir.isEmpty())	  // first app launch, settings file absent
	{
		dir = QDir::homePath();
	}

	return dir;
}

void QtFileDialog::saveFilePickerLocation(const FilePath& path)
{
	if (!path.empty())
	{
		std::shared_ptr<ApplicationSettings> settings = ApplicationSettings::getInstance();
		settings->setLastFilepickerLocation(path);
		settings->save();
	}
}
