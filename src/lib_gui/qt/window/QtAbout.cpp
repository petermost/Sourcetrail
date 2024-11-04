#include "QtAbout.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "QtDeviceScaledPixmap.h"
#include "ResourcePaths.h"
#include "SqliteIndexStorage.h"
#include "Version.h"
#include "utilityQt.h"

using namespace utility;
using namespace std::string_literals;

QtAbout::QtAbout(QWidget* parent): QtWindow(false, parent) {}

QSize QtAbout::sizeHint() const
{
	return QSize(450, 480);
}

void QtAbout::setupAbout()
{
	setStyleSheet(
		utility::getStyleSheet(ResourcePaths::getGuiDirectoryPath().concatenate(L"about/about.css"))
			.c_str());

	QVBoxLayout* windowLayout = new QVBoxLayout();
	windowLayout->setContentsMargins(10, 10, 10, 0);
	windowLayout->setSpacing(1);
	m_content->setLayout(windowLayout);

	{
		QtDeviceScaledPixmap sourcetrailLogo(QString::fromStdWString(
			ResourcePaths::getGuiDirectoryPath().wstr() + L"window/numbatui.png"));
		sourcetrailLogo.scaleToHeight(150);
		QLabel* sourcetrailLogoLabel = new QLabel(this);
		sourcetrailLogoLabel->setPixmap(sourcetrailLogo.pixmap());
		sourcetrailLogoLabel->resize(
			static_cast<int>(sourcetrailLogo.width()), static_cast<int>(sourcetrailLogo.height()));
		windowLayout->addWidget(
			sourcetrailLogoLabel, 0, Qt::Alignment(Qt::AlignmentFlag::AlignHCenter));
	}

	windowLayout->addSpacing(10);

	{
		QLabel* versionLabel = new QLabel(("Version " + Version::getApplicationVersion().toDisplayString()).c_str(), this);
		windowLayout->addWidget(versionLabel, 0, Qt::Alignment(Qt::AlignmentFlag::AlignHCenter));
	}

	{
		QLabel* dbVersionLabel = new QLabel("Database Version " + QString::number(SqliteIndexStorage::getStorageVersion()),
			this);
		windowLayout->addWidget(dbVersionLabel, 0, Qt::Alignment(Qt::AlignmentFlag::AlignHCenter));
	}

	windowLayout->addStretch();

	{
		QHBoxLayout* layoutHorz1 = new QHBoxLayout();
		windowLayout->addLayout(layoutHorz1);

		layoutHorz1->addStretch();

		QLabel* developerLabel = new QLabel(
<<<<<<< HEAD
			"<b>Authors:</b><br />"
			"<a href=\"https://github.com/petermost/Sourcetrail/blob/master/unused_coati_software_files/AUTHORS.txt\" "
			"style=\"color: white;\">The Coati Software Developer</a><br />"
			"<br />"
			"<b>Maintainer:</b><br />"
			"Peter Most<br />"
			"<br />"
			"<b>Repository:</b><br />"
			"<a href=\"https://github.com/petermost/Sourcetrail\" "
			"style=\"color: white;\">github.com/petermost/Sourcetrail</a><br />");
=======
			QString::fromStdString("<br /><br />"
								   "Developed by Quarkslab<br />"));
>>>>>>> 09ccbe42a1120f7185e91e13d9d2b8583217be7f

		developerLabel->setObjectName(QStringLiteral("small"));
		developerLabel->setOpenExternalLinks(true);
		layoutHorz1->addWidget(developerLabel);

		layoutHorz1->addStretch();
	}

	windowLayout->addStretch();
<<<<<<< HEAD
=======

	{
		QLabel* acknowledgementsLabel = new QLabel(QString::fromStdString(
			"<b>Acknowledgements:</b><br />"
			"NumbatUI is forked from <b><a href=\"https://github.com/CoatiSoftware/Sourcetrail\" "
			"style=\"color:white;\">Sourcetrail</a></b>, originally developed by:<br /><br />"
			"Manuel Dobusch<br />"
			"Eberhard Gräther<br />"
			"Malte Langkabel<br />"
			"Viktoria Pfausler<br />"
			"Andreas Stallinger<br />"));
		acknowledgementsLabel->setObjectName(QStringLiteral("small"));
		acknowledgementsLabel->setWordWrap(true);
		acknowledgementsLabel->setOpenExternalLinks(true);
		windowLayout->addWidget(acknowledgementsLabel);
		windowLayout->addSpacing(10);
	}

	{
		QLabel* webLabel = new QLabel(
			"<b>Repository: <a href=\"https://github.com/quarkslab/Numbat-UI\" "
			"style=\"color: "
			"white;\">https://github.com/quarkslab/Numbat-UI</a></b>",
			this);
		webLabel->setObjectName(QStringLiteral("small"));
		webLabel->setOpenExternalLinks(true);
		windowLayout->addWidget(webLabel);
		windowLayout->addSpacing(10);
	}
>>>>>>> 09ccbe42a1120f7185e91e13d9d2b8583217be7f
}
