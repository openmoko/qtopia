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

#include "otaimpl.h"
#include <qimage.h>

static void read_ota_image(QImageIO* iio)
{
    QIODevice  *d = iio->ioDevice();
    int ch = -1, infoField, width, height, depth;
    int x, y, bits;

    // Read the OTA header.
    infoField = d->getch();
    if ( infoField == -1 )
	return;
    if ( (infoField & 0x80) != 0 ) {
	// Skip extension fields, which aren't defined yet.
	do
	{
	    ch = d->getch();
	}
	while ( ch != -1 && (ch & 0x80) != 0 );
    }

    // Read the width, height, and depth of the image.
    if ( (infoField & 0x10) != 0 ) {
	ch = d->getch();
	if ( ch == -1 )
	    return;
	width = ch * 256;
	ch = d->getch();
	if ( ch == -1 )
	    return;
	width += ch;
	ch = d->getch();
	if ( ch == -1 )
	    return;
	height = ch * 256;
	ch = d->getch();
	if ( ch == -1 )
	    return;
	height += ch;
    } else {
	width = d->getch();
	if ( width == - 1 )
	    return;
	height = d->getch();
	if ( height == - 1 )
	    return;
    }
    depth = d->getch();
    if ( depth == -1 )
	return;
    if ( width <= 0 || height <= 0 )
	return;

    // Decode the first image, which is the black and white version.
    // The spec is unclear as to how color images are encoded, so we
    // will add that once a better specification is available.
    //
    // OTA bitmaps pack the pixels very densely, with no padding pixels
    // at the end of lines.  Black=1, White=0.
    QImage image(width,height,1,2,QImage::BigEndian);
    image.setColor(1,qRgb(0,0,0));
    image.setColor(0,qRgb(255,255,255));
    bits = 0;
    for ( y = 0; y < height; ++y ) {
	for ( x = 0; x < width; ++ x ) {
	    if ( bits <= 0 ) {
		ch = d->getch();
		if ( ch == -1 )
		    return;
		bits = 8;
	    }
	    if ( (ch & 0x80) != 0 )
		image.setPixel( x, y, 1 );
	    else
		image.setPixel( x, y, 0 );
	    ch <<= 1;
	    --bits;
	}
    }

    // Return the completed image to the caller.
    iio->setImage(image);
    iio->setStatus(0);
}

OtaImpl::OtaImpl()
    : ref(0)
{
}

OtaImpl::~OtaImpl()
{
}

QStringList OtaImpl::keys() const
{
    return QStringList() << QString("OTA");
}

bool OtaImpl::installIOHandler( const QString &format )
{
    if ( format == "OTA" ) {
	// OTA images usually start with a zero byte indicating header flags,
	// followed by the non-zero width value.  This is the most common
	// case, but it doesn't handle all possibilities.
	QImageIO::defineIOHandler("OTA", "^\001[\002-\377]", 0, read_ota_image, 0);
	return TRUE;
    }
    return FALSE;
}



QRESULT OtaImpl::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
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
    Q_CREATE_INSTANCE( OtaImpl )
}
