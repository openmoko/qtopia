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
#ifndef IMAGE_UTIL_H
#define IMAGE_UTIL_H


#include <qwidget.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qbitmap.h>


// Qt 2.3 lacks the QImage ScaleMode using in smoothScale that is now in Qt 3.0
// We fall back to handling this ourselves
enum ScaleMode { ScaleFree, ScaleMin, ScaleMax };
void scaleSize( int &reqW, int &reqH, int imgW, int imgH, ScaleMode mode );


void changeTextColor( QWidget *w );
QPixmap combineImageWithBackground( const QImage &img, const QPixmap &bg, const QPoint &offset );
QPixmap maskPixToMask( const QPixmap &pix, const QBitmap &mask );
QImage fastScale( const QString &file, int w, int h );


#endif // IMAGE_UTIL_H

