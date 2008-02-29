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

#ifdef QWS
#include "fontmanager.h"
#include <qfile.h>
#include <stdlib.h>
#include <qgfx_qws.h>
#ifdef Q_OS_WIN32
#include <qtopia/qpeapplication.h>
#include <qdir.h>
#endif



/*! internal
    Determine where unicode fonts are installed
*/
static QString unicodeFontDirPath()
{
#ifndef Q_OS_WIN32
    QString fontDir = getenv("QTDIR") + QString("/lib/fonts/");
#else
    QString fontDir, qtDir;
    if ( getenv("QTDIR") )
	qtDir = QString(getenv("QTDIR")).stripWhiteSpace();
    if ( qtDir.isEmpty() )
	fontDir = QPEApplication::qpeDir();
    else
	fontDir = qtDir + QDir::separator();
    QDir d(fontDir + "lib/fonts/");
    if (d.exists())
	fontDir.append("lib/fonts/");
    else
	fontDir.append("dist/embedded/lib/fonts/"); // internal development
#endif
    return fontDir;
}

/*
    QFontInfo doesn't work in QWS at the moment,
    otherwise we would just have used that to check
    the real values
    
     For now, there are only two Unicode fonts in
     the known universe...
    
*/

bool FontManager::hasUnicodeFont()
{
    QString fontDir = unicodeFontDirPath();
    QString suffix;
    if ( qt_screen->isTransformed() ) {
	suffix += "_t";
	QPoint a = qt_screen->mapToDevice(QPoint(0,0),QSize(2,2));
	QPoint b = qt_screen->mapToDevice(QPoint(1,1),QSize(2,2));
	suffix += QString::number( a.x()*8+a.y()*4+(1-b.x())*2+(1-b.y()) );
    }
    suffix += ".qpf";

    return QFile::exists( fontDir+"cyberbit_120_50"+suffix )
	|| QFile::exists( fontDir+"unifont_160_50"+suffix ) ||
	QFile::exists( fontDir+"arial_140_50" + suffix );
}

QFont FontManager::unicodeFont( Spacing sp )
{
    QString key;
    QString fontName;
    QString fontDir = unicodeFontDirPath();

    int size;
    if ( sp == Proportional ) {
	fontName = "Arial";
	size=14;
	key = "arial_140_50";
    } else {
	fontName = "Unifont";
	size=16;
	key = "unifont_160_50";
    }
	
    QString suffix;
    if ( qt_screen->isTransformed() ) {
	suffix += "_t";
	QPoint a = qt_screen->mapToDevice(QPoint(0,0),QSize(2,2));
	QPoint b = qt_screen->mapToDevice(QPoint(1,1),QSize(2,2));
	suffix += QString::number( a.x()*8+a.y()*4+(1-b.x())*2+(1-b.y()) );
    }
    suffix += ".qpf";

    // if we cannot find it, try the other one
	
    if ( !QFile::exists(fontDir+key+suffix) ) {
	key = (sp == Fixed ) ? "arial_140_50" : "unifont_160_50";
	if ( QFile::exists(fontDir+key+suffix) ) {
	    fontName = (sp == Fixed) ? "Arial" : "Unifont";
	    size = (sp == Fixed) ? 14 : 16;
	} else {
	    key = "cyberbit_120_50";
	    if ( QFile::exists(fontDir+key+suffix) ) {
		fontName = "Cyberbit";
		size = 12;
	    } else {
		fontName = "Helvetica"; // No tr
		size = 14;
	    }
	}
    }
    return QFont(fontName,size);
}
#endif
