/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
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

