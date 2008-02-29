/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qtopianamespace.h>
#include <qthumbnail.h>

#include <QLabel>
#include <QPixmap>
#include <QPainter>
#include <QImage>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QLayout>
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include "versioninfo.h"

VersionInfo::VersionInfo( QWidget *parent, Qt::WFlags f )
    : QWidget( parent, f )
{
    //QTimer::singleShot(1, this, SLOT(init()));
    init();
}

VersionInfo::~VersionInfo()
{
}

void VersionInfo::init()
{
#ifdef SYSINFO_GEEK_MODE
    //setMinimumSize(QSize(16,350));
#endif
    QFont boldFont = this->font();
    boldFont.setBold(true);

    QDesktopWidget *desktop = QApplication::desktop();
    double imageScale = ((double)desktop->availableGeometry(desktop->screenNumber(this)).width())/400.0;
    if (imageScale > 1.0)
        imageScale = 1.0;
    int imageSize = (int)(55 * imageScale);
    int finalSize = (int)(60 * imageScale);

    QVBoxLayout *vBoxLayout = new QVBoxLayout(this);

    QGridLayout *gridLayout1 = new QGridLayout;

    QLabel *qtopiaLogo = new QLabel(this);
    QThumbnail thumbnail(":image/qpe-logo");
    qtopiaLogo->setPixmap(thumbnail.pixmap(QSize(imageSize, imageSize)));
    qtopiaLogo->setFixedSize( finalSize, finalSize );
    gridLayout1->addWidget(qtopiaLogo, 0, 0, 1, 1);

    QSpacerItem *spacerItem1 = new QSpacerItem(20, 90, QSizePolicy::Minimum, QSizePolicy::Expanding);
    gridLayout1->addItem(spacerItem1, 1, 0, 1, 1);

    QVBoxLayout *vBoxLayout1 = new QVBoxLayout;
    QLabel *qtopiaName = new QLabel(this);
    qtopiaName->setFont(boldFont);
    qtopiaName->setText(tr("Qtopia"));
    vBoxLayout1->addWidget(qtopiaName);

    QLabel *qtopiaVersion = new QLabel(this);
    qtopiaVersion->setWordWrap(true);
    qtopiaVersion->setText(tr("Version:") + " " + Qtopia::version());
    vBoxLayout1->addWidget(qtopiaVersion);
    QLabel *qtopiaSpacerLabel1 = new QLabel(this);
    vBoxLayout1->addWidget(qtopiaSpacerLabel1);

    QLabel *qtopiaCopyright = new QLabel(this);
    qtopiaCopyright->setWordWrap(true);
    qtopiaCopyright->setTextFormat(Qt::PlainText);
    qtopiaCopyright->setText(tr( "Copyright \251 %1 %2", "%1 = 'year',%2 = 'company'" ).arg(2006).arg("Trolltech ASA"));
//    qtopiaCopyright->setText(tr( "Copyright (c) %1 %2", "%1 = 'year',%2 = 'company'" ).arg(2006).arg("Trolltech ASA"));
    vBoxLayout1->addWidget(qtopiaCopyright);

#ifdef SYSINFO_GEEK_MODE
    QLabel *qtopiaSpacerLabel2 = new QLabel(this);
    vBoxLayout1->addWidget(qtopiaSpacerLabel2);

    QLabel *qtopiaBuild = new QLabel(this);
    qtopiaBuild->setWordWrap(true);
    qtopiaBuild->setText(tr("Built by %1","1=name@host").arg(BUILDER));
    vBoxLayout1->addWidget(qtopiaBuild);

    qtopiaBuild = new QLabel(this);
    qtopiaBuild->setWordWrap(true);
    qtopiaBuild->setText(tr("Built on %1","1=date").arg(__DATE__));
    vBoxLayout1->addWidget(qtopiaBuild);
#endif

    QSpacerItem *spacerItem = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vBoxLayout1->addItem(spacerItem);
    gridLayout1->addLayout(vBoxLayout1, 0, 1, 2, 1);
    vBoxLayout->addLayout(gridLayout1);

#ifdef SYSINFO_GEEK_MODE
    QGridLayout *gridLayout2 = new QGridLayout;
    QLabel *linuxLogo = new QLabel(this);
    QThumbnail thumbnail2(":image/tux-logo");
    linuxLogo->setPixmap(thumbnail2.pixmap(QSize(imageSize, imageSize)));
    linuxLogo->setFixedSize( finalSize, finalSize );
    gridLayout2->addWidget(linuxLogo, 0, 0, 1, 1);

    QSpacerItem *spacerItem2 = new QSpacerItem(20, 126, QSizePolicy::Minimum, QSizePolicy::Expanding);
    gridLayout2->addItem(spacerItem2, 1, 0, 1, 1);

    QVBoxLayout *vBoxLayout2 = new QVBoxLayout;
    QLabel *linuxName = new QLabel(this);
    linuxName->setFont(boldFont);
    linuxName->setText(tr("Linux Kernel"));
    vBoxLayout2->addWidget(linuxName);

    QString kernelVersionString;
    QString compiledByString;
    QFile file("/proc/version");
    if(file.open(QFile::ReadOnly))
    {
        QTextStream t( &file );
        QString v;
        t >> v; t >> v; t >> v;
        kernelVersionString = v.left( 20 );
        t >> v;
        compiledByString = v;
        file.close();
    }
    QLabel *linuxVersion = new QLabel(this);
    linuxVersion->setWordWrap(true);
    linuxVersion->setText(tr("Version:")+ " " + kernelVersionString);
    vBoxLayout2->addWidget(linuxVersion);

    if (desktop->screenGeometry(desktop->screenNumber(this)).height() >= 220)
    {
        QLabel *linuxSpacerLabel1 = new QLabel(this);
        vBoxLayout2->addWidget(linuxSpacerLabel1);
    }

    QLabel *linuxCompiledBy = new QLabel(this);
    linuxCompiledBy->setWordWrap(true);
    linuxCompiledBy->setText(tr("Compiled by:") + " " + compiledByString);
    vBoxLayout2->addWidget(linuxCompiledBy);

    QSpacerItem *spacerItem3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vBoxLayout2->addItem(spacerItem3);

    gridLayout2->addLayout(vBoxLayout2, 0, 1, 2, 1);
    vBoxLayout->addLayout(gridLayout2);
#endif
}