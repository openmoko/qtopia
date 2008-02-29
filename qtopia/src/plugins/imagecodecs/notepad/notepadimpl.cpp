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

#include "notepadimpl.h"
#include <qimage.h>

static int read_int(QIODevice *d)
{
    return (d->getch()<<24)+(d->getch()<<16)+(d->getch()<<8)+d->getch();
}

static void read_npad_image(QImageIO* iio)
{
    QIODevice  *d = iio->ioDevice();
    char header[12];
    d->readBlock(header,12);
    char date[14];
    d->readBlock(date,14);
    d->readBlock(date,14);
    short flags = (d->getch() << 8) + d->getch();
    if ( flags & 4 ) {
	// Alarm date
	d->readBlock(date,14);
    }
    if ( flags & 2 ) {
	QString name;
	int ch = d->getch();
	while ( ch>0 ) {
	    name+=QChar(ch);
	    ch = d->getch();
	}
	if ( !(name.length()&1) )
	    d->getch();
    }
    if ( flags & 1 ) {
	read_int(d);		/* int bodylen = */
	int width = read_int(d);
	int height = read_int(d);
	read_int(d);		/* int l1 = */
	read_int(d);		/* int l2 = */
	int datalen = read_int(d);
	width = (width+15)/16*16;
	if ( width > 0 && height > 0 ) {
	    QImage image(width,height,1,2,QImage::BigEndian);
	    image.setColor(1,qRgb(0,0,0));
	    image.setColor(0,qRgb(255,255,255));
	    int n = image.numBytes();
	    uchar* cursor = image.bits();
	    while ( datalen > 0 && n>0 ) {
		int repeat = d->getch();
		int data = d->getch();
		while ( repeat-- && n-->0 )
		    *cursor++ = (uchar)data;
		datalen-=2;
	    }
	    iio->setImage(image);
	    iio->setStatus(0);
	}
    }
}

/*
static void write_npad_image(QImageIO* iio)
{
}
*/

NotePadImpl::NotePadImpl()
    : ref(0)
{
}

NotePadImpl::~NotePadImpl()
{
}

QStringList NotePadImpl::keys() const
{
    return QStringList() << QString("NPD");
}

bool NotePadImpl::installIOHandler( const QString &format )
{
    if ( format == "NPD" ) {
	QImageIO::defineIOHandler("NPD", "^npad", 0, read_npad_image, 0);
	return TRUE;
    }
    return FALSE;
}



QRESULT NotePadImpl::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
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
    Q_CREATE_INSTANCE( NotePadImpl )
}
