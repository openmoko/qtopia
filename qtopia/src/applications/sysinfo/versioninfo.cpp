/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include <qtopia/resource.h>
#include <qtopia/global.h>

#include <qlabel.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qimage.h>
#include <qtimer.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qlayout.h>
#include <qapplication.h>
#include "versioninfo.h"

VersionInfo::VersionInfo( QWidget *parent, const char *name, Qt::WFlags f )
    : QWidget( parent, name, f )
{
#ifdef SYSINFO_GEEK_MODE
    //setMinimumSize(QSize(16,350));
#endif
    QFont normalFont = this->font();
    QFont boldFont = this->font();
    boldFont.setBold(true);

    double imageScale = ((double)QApplication::desktop()->width())/400.0;
    if (imageScale > 1.0)
	imageScale = 1.0;
    int imageSize = (int)(55 * imageScale);
    int finalSize = (int)(60 * imageScale);
    if (normalFont.pointSize() != -1)
    {
	normalFont.setPointSize((int)(normalFont.pointSize() * imageScale));
	boldFont.setPointSize((int)(boldFont.pointSize() * imageScale));
    }
    else
    {
	normalFont.setPixelSize((int)(normalFont.pixelSize() * imageScale));
	boldFont.setPixelSize((int)(boldFont.pixelSize() * imageScale));
    }
    this->setFont(normalFont);

    QVBoxLayout *vBoxLayout = new QVBoxLayout(this);

    QGridLayout *gridLayout1 = new QGridLayout;

    QLabel *qtopiaLogo = new QLabel(this);
    QImage logo1 = Resource::loadImage( "qpe-logo" );
    logo1 = logo1.smoothScale( imageSize , imageSize );
    QPixmap logo1Pixmap;
    logo1Pixmap.convertFromImage( logo1 );
    qtopiaLogo->setPixmap(logo1Pixmap);
    qtopiaLogo->setFixedSize( finalSize, finalSize );
    gridLayout1->addWidget(qtopiaLogo, 0, 0);

    QSpacerItem *spacerItem1 = new QSpacerItem(20, 90, QSizePolicy::Minimum, QSizePolicy::Expanding);
    gridLayout1->addItem(spacerItem1, 1, 0);

    QVBoxLayout *vBoxLayout1 = new QVBoxLayout;
    QLabel *qtopiaName = new QLabel(this);
    qtopiaName->setFont(boldFont);
    qtopiaName->setText(tr("Qtopia"));
    vBoxLayout1->addWidget(qtopiaName);

    QLabel *qtopiaVersion = new QLabel(this);
    qtopiaVersion->setAlignment(int(QLabel::WordBreak | QLabel::AlignVCenter));
    qtopiaVersion->setText(tr("Version: ") + Global::version());
    vBoxLayout1->addWidget(qtopiaVersion);
    QLabel *qtopiaSpacerLabel1 = new QLabel(this);
    vBoxLayout1->addWidget(qtopiaSpacerLabel1);

    QLabel *qtopiaCopyright = new QLabel(this);
    qtopiaCopyright->setAlignment(int(QLabel::WordBreak | QLabel::AlignVCenter));
    qtopiaCopyright->setTextFormat(Qt::PlainText);
    qtopiaCopyright->setText(tr( "Copyright \251 %1 %2", "%1 = 'year',%2 = 'company'" ).arg(2005).arg("Trolltech AS"));
    vBoxLayout1->addWidget(qtopiaCopyright);

#ifdef SYSINFO_GEEK_MODE
    QLabel *qtopiaSpacerLabel2 = new QLabel(this);
    vBoxLayout1->addWidget(qtopiaSpacerLabel2);

    QLabel *qtopiaBuild = new QLabel(this);
    qtopiaBuild->setAlignment(int(QLabel::WordBreak | QLabel::AlignVCenter));
    qtopiaBuild->setText(tr("Built on: ") + __DATE__);
    vBoxLayout1->addWidget(qtopiaBuild);
#endif

    QSpacerItem *spacerItem = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vBoxLayout1->addItem(spacerItem);
    gridLayout1->addMultiCellLayout(vBoxLayout1, 0, 1, 1, 1);
    vBoxLayout->addLayout(gridLayout1);

#ifdef SYSINFO_GEEK_MODE
    QGridLayout *gridLayout2 = new QGridLayout;
    QLabel *linuxLogo = new QLabel(this);
    QPixmap logo2Pixmap( finalSize , finalSize );
    QColor bgColor = colorGroup().background();
    QPainter painter( &logo2Pixmap );
    painter.fillRect( QRect( 0, 0, finalSize, finalSize ), QBrush( bgColor ) );
    QImage logo2 = Resource::loadImage( "tux-logo" );
    logo2 = logo2.smoothScale( imageSize, imageSize );
    painter.drawImage( 0, 0, logo2 );
    painter.end();
    linuxLogo->setPixmap(logo2Pixmap);
    linuxLogo->setFixedSize( finalSize, finalSize );
    gridLayout2->addWidget(linuxLogo, 0, 0);

    QSpacerItem *spacerItem2 = new QSpacerItem(20, 126, QSizePolicy::Minimum, QSizePolicy::Expanding);
    gridLayout2->addItem(spacerItem2, 1, 0);

    QVBoxLayout *vBoxLayout2 = new QVBoxLayout;
    QLabel *linuxName = new QLabel(this);
    linuxName->setFont(boldFont);
    linuxName->setText(tr("Linux Kernel"));
    vBoxLayout2->addWidget(linuxName);

    QString kernelVersionString;
    QString compiledByString;
    QFile file("/proc/version");
    if(file.open(IO_ReadOnly))
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
    linuxVersion->setAlignment(int(QLabel::WordBreak | QLabel::AlignVCenter));
    linuxVersion->setText(tr("Version: ") + kernelVersionString);
    vBoxLayout2->addWidget(linuxVersion);

    if (QApplication::desktop()->height() >= 220)
    {
	QLabel *linuxSpacerLabel1 = new QLabel(this);
	vBoxLayout2->addWidget(linuxSpacerLabel1);
    }

    QLabel *linuxCompiledBy = new QLabel(this);
    linuxCompiledBy->setAlignment(int(QLabel::WordBreak | QLabel::AlignVCenter));
    linuxCompiledBy->setText(tr("Compiled by: ") + compiledByString);
    vBoxLayout2->addWidget(linuxCompiledBy);

    QSpacerItem *spacerItem3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vBoxLayout2->addItem(spacerItem3);

    gridLayout2->addMultiCellLayout(vBoxLayout2, 0, 1, 1, 1);
    vBoxLayout->addLayout(gridLayout2);
#endif
}

VersionInfo::~VersionInfo()
{
}

