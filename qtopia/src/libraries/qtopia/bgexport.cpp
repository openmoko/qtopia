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

#include "bgexport_p.h"
#include <qcopchannel_qws.h>
#include <qpixmap.h>
#include <qpixmapcache.h>
#include <qapplication.h>

class BgExportPrivate
{
public:
    QPixmap bg;
    QPixmap base;
    QPixmap alt;
    QPalette pal;
    bool avail;
};

BgExport::BgExport(QObject *parent)
    : QObject(parent)
{
    QCopChannel* sysChannel = new QCopChannel( "QPE/System", this );
    connect( sysChannel, SIGNAL(received(const QCString&,const QByteArray&)),
	    this, SLOT(sysMessage(const QCString&,const QByteArray&)) );
    d = new BgExportPrivate;
    getPixmaps();
}

BgExport::~BgExport()
{
}

const QPixmap &BgExport::background() const
{
    return d->bg;
}

const QPixmap &BgExport::base() const
{
    return d->base;
}

const QPixmap &BgExport::alternateBase() const
{
    return d->alt;
}

bool BgExport::isAvailable() const
{
    return d->avail;
}

void BgExport::sysMessage(const QCString &msg, const QByteArray&)
{
    if (msg == "homeScreenChanged()") {
	getPixmaps();
	emit changed();
    }
}

void BgExport::getPixmaps()
{
    d->avail = TRUE;
    QPixmap bgState;
    if (QPixmapCache::find("_$QPE_BGSTATE", bgState)) {
	if (*bgState.scanLine(0)) {
	    if (!QPixmapCache::find("_$QPE_BG", d->bg))
		d->avail = FALSE;
	    if (!QPixmapCache::find("_$QPE_BASE", d->base))
		d->avail = FALSE;
	    if (!QPixmapCache::find("_$QPE_ALT", d->alt))
		d->avail = FALSE;
	} else {
	    d->avail = FALSE;
	}
    } else {
	d->avail = FALSE;
    }

    if (!d->avail) {
	d->bg = QPixmap();
	d->base = QPixmap();
	d->alt = QPixmap();
    }
}

