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

#include "wbmpimpl.h"
#include <qimage.h>

static int read_int(QIODevice *d)
{
    int value = 0;
    int ch;
    while ( (ch = d->getch()) != -1 ) {
	value = (value << 7) | (ch & 0x7F);
	if ( (ch & 0x80) == 0 ) {
	    break;
	}
    }
    return value;
}

static void read_wbmp_image(QImageIO* iio)
{
    QIODevice  *d = iio->ioDevice();
    int ch, width, height;

    // Verify that we have a simple header with no extension fields
    // (fix this if we ever come across a WBMP with extension headers).
    if ( read_int( d ) != 0 ) {
	return;
    }
    ch = d->getch();
    if ( ch != 0 ) {
	return;
    }

    // Read the width and height values.
    width = read_int( d );
    height = read_int( d );
    if ( width <= 0 || height <= 0 ) {
	return;
    }

    // Create the image.  In the original, we have black=0, white=1.
    // However, there is code elsewhere in Qtopia that assumes that
    // bitmaps have black=1, white=0.  We therefore invert the image.
    QImage image(width,height,1,2,QImage::BigEndian);
    image.setColor(1,qRgb(0,0,0));
    image.setColor(0,qRgb(255,255,255));

    // Read the bits that make up the image.
    for ( int y = 0; y < height; ++y ) {
	d->readBlock( (char *)(image.scanLine(y)), (width + 7) / 8 );
    }
    image.invertPixels();

    // Return the completed image to the caller.
    iio->setImage(image);
    iio->setStatus(0);
}

WBmpImpl::WBmpImpl()
    : ref(0)
{
}

WBmpImpl::~WBmpImpl()
{
}

QStringList WBmpImpl::keys() const
{
    return QStringList() << QString("WBMP");
}

bool WBmpImpl::installIOHandler( const QString &format )
{
    if ( format == "WBMP" ) {
	// WBMP images start with a zero byte indicating the type,
	// and a "FixHeaderField" byte which is normally also zero.
	// WBMP's theoretically have parameters that can cause
	// "FixHeaderField" to be non-zero, but no one uses them.
	// Note QImageIO::imageFormat turns zero into '\001'.
	QImageIO::defineIOHandler("WBMP", "^\001\001", 0, read_wbmp_image, 0);
	return TRUE;
    }
    return FALSE;
}



QRESULT WBmpImpl::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( uuid == IID_QUnknown )
	*iface = this;
    else if ( uuid == IID_QtopiaImageCodec )
	*iface = this;

    if ( *iface )
	(*iface)->addRef();
    return (*iface) ? QS_OK : QS_FALSE;
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( WBmpImpl )
}
