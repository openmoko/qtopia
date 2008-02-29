/**********************************************************************
** Copyright (C) 2000-2004 Trolltech AS.  All rights reserved.
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

VersionInfo::VersionInfo( QWidget *parent, const char *name, WFlags f )
    : QWidget( parent, name, f )
{
    if (QApplication::desktop()->width() > 240)
	setMinimumSize( 200, 150 );

    QVBoxLayout *vb = new QVBoxLayout( this, 4 );
    QString lineBreak;
#ifdef QTOPIA_PHONE
    lineBreak = "<br>";
#else
    Q_UNUSED(lineBreak);
#endif

#ifdef SYSINFO_GEEK_MODE
    QString kernelVersionString;
    QFile file( "/proc/version" );
    if ( file.open( IO_ReadOnly ) ) {
	QTextStream t( &file );
	QString v;
	t >> v; t >> v; t >> v;
	v = v.left( 20 );
	kernelVersionString = tr( "<b>Linux Kernel</b><p>Version:%1 " ).arg(lineBreak) 
                            + v + "<p>";
	t >> v;
	kernelVersionString += tr( "Compiled by: " ) + v;
	file.close();
    }
#endif

    QString qtopiaVersionString;
    qtopiaVersionString = tr( "<b>Qtopia</b><p>Version: " ) + Global::version() + "<p>";
    qtopiaVersionString += tr( "Copyright &copy; %1 %2%3", "%1 = 'year', %2 = 'conditional linebreak - ignore for translation', %3 = 'company'" )
			    .arg(2004).arg(lineBreak)
			    .arg("Trolltech&nbsp;AS");
#ifdef SYSINFO_GEEK_MODE
    qtopiaVersionString += "<p>" + tr( "Built on: %1" ).arg(lineBreak) + __DATE__;
#endif

    
    QHBoxLayout *hb1 = new QHBoxLayout( vb );
    hb1->setSpacing( 2 );

    double imageScale = ((double)QApplication::desktop()->width())/290.0;
    if (imageScale > 1.0)
	imageScale = 1.0;
    int imageSize = (int)(55 * imageScale);
    int finalSize = (int)(60 * imageScale);
    QLabel *qtopiaLogo = new QLabel( this );
    QImage logo1 = Resource::loadImage( "qpe-logo" );
    logo1 = logo1.smoothScale( imageSize , imageSize );
    QPixmap logo1Pixmap;
    logo1Pixmap.convertFromImage( logo1 );
    qtopiaLogo->setPixmap( logo1Pixmap );
    qtopiaLogo->setFixedSize( finalSize, finalSize );
    hb1->addWidget( qtopiaLogo, 0, Qt::AlignTop + Qt::AlignLeft );
    
    QLabel *qtopiaVersion = new QLabel( this );
    qtopiaVersion->setText( qtopiaVersionString  );
    hb1->addWidget( qtopiaVersion, 1, Qt::AlignTop + Qt::AlignLeft );
    hb1->setSpacing( 2 );
    
#ifdef SYSINFO_GEEK_MODE
    QHBoxLayout *hb2 = new QHBoxLayout( vb );
    QLabel *linuxLogo = new QLabel( this );

    // Need to do this extra qpainter code with this image becuase for some
    // reason it doesn't alpha belnd if directly converted to a pixmap
    QPixmap logo2Pixmap( finalSize , finalSize );
    QColor bgColor = colorGroup().background();
    QPainter painter( &logo2Pixmap );
    painter.fillRect( QRect( 0, 0, finalSize, finalSize ), QBrush( bgColor ) );
    QImage logo2 = Resource::loadImage( "tux-logo" );
    logo2 = logo2.smoothScale( imageSize, imageSize );
    painter.drawImage( 0, 0, logo2 );
    painter.end();
    linuxLogo->setPixmap( logo2Pixmap );
    linuxLogo->setFixedSize( finalSize, finalSize );
    hb2->addWidget( linuxLogo, 0, Qt::AlignTop + Qt::AlignLeft );
    
    QLabel *kernelVersion = new QLabel( this );
    kernelVersion->setText( kernelVersionString );
    hb2->addWidget( kernelVersion, 1, Qt::AlignTop + Qt::AlignLeft );
#endif
}

VersionInfo::~VersionInfo()
{
}

