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
#include "mountmon.h"

#include <qtopia/resource.h>
#include <qtopia/storage.h>

#include <qcopchannel_qws.h>

#include <qpopupmenu.h>
#include <qpainter.h>

#include <stdlib.h>

MountMon::MountMon( QWidget *parent ) : QWidget( parent ),
	// ##### For now, we're just using this as an SD-card ejector.
	pm( Resource::loadPixmap( "mountmon/sdcard" ) ),
	cardIn( FALSE )
{
    si = new StorageInfo(this);
    connect(si, SIGNAL(disksChanged()), this, SLOT(refresh()));
    setFixedWidth(12);
    hide();
    refresh();
}

MountMon::~MountMon()
{
}

void MountMon::refresh()
{
    bool in = FALSE;
    const QList<FileSystem>& fsl = si->fileSystems();
    for (QListIterator<FileSystem> i(fsl); i.current(); ++i) {
	const FileSystem* fs = *i;
	// ##### For now, we're just using this as an SD-card ejector.
	if ( fs->isRemovable() && fs->name().left(2) == "SD" ) {
	    in = TRUE;
	}
    }
    if ( in != cardIn ) {
	cardIn = in;
	if ( cardIn )
	    show();
	else
	    hide();
    }
}

void MountMon::mousePressEvent( QMouseEvent * )
{
    // ##### For now, we're just using this as an SD-card ejector.

    QPopupMenu menu;
    menu.insertItem( tr("Eject SD-card"), 0 );
    QPoint p = mapToGlobal( QPoint(0, -menu.sizeHint().height()) );

    if ( menu.exec( p, 1 ) == 0 ) {
	system( "/etc/sdcontrol eject" );
	refresh();
    }
}

void MountMon::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    if ( cardIn )
	p.drawPixmap( (width()-pm.width())/2, (height()-pm.height())/2, pm );
}


