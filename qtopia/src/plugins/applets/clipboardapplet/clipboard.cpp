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

#include "clipboard.h"

#include <qtopia/resource.h>

#include <qpainter.h>
#include <qpopupmenu.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qwindowsystem_qws.h>

#include <qtopia/applnk.h>

//===========================================================================

ClipboardApplet::ClipboardApplet( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    clipboardPixmap = Resource::loadIconSet("clipboard").pixmap(QIconSet::Small, true);

    setFixedWidth(AppLnk::smallIconSize());
//    setFixedHeight(AppLnk::smallIconSize());

    menu = 0;
}

ClipboardApplet::~ClipboardApplet()
{
}

void ClipboardApplet::mousePressEvent( QMouseEvent *)
{
    if ( !menu ) {
	menu = new QPopupMenu(this);
	menu->insertItem(Resource::loadIconSet("cut"), tr("Cut"),0);
	menu->insertItem(Resource::loadIconSet("copy"), tr("Copy"),1);
	menu->insertItem(Resource::loadIconSet("paste"), tr("Paste"),2);
	connect(menu, SIGNAL(activated(int)), this, SLOT(action(int)));
	connect(qApp->clipboard(), SIGNAL(dataChanged()), this, SLOT(clipboardChanged()) );
	clipboardChanged();
    }
    QPoint curPos = mapToGlobal( QPoint(0,0) );
    QSize sh = menu->sizeHint();
    menu->popup( curPos-QPoint((sh.width()-width())/2,sh.height()) );
}

void ClipboardApplet::action(int i)
{
    ushort unicode=0;
    int scan=0;

    if ( i == 0 )
	{ unicode='X'-'@'; scan=Key_X; } // Cut
    else if ( i == 1 )
	{ unicode='C'-'@'; scan=Key_C; } // Copy
    else if ( i == 2 )
	{ unicode='V'-'@'; scan=Key_V; } // Paste

    if ( scan ) {
	qwsServer->processKeyEvent( unicode, scan, ControlButton, TRUE, FALSE );
	qwsServer->processKeyEvent( unicode, scan, ControlButton, FALSE, FALSE );
    }
}

void ClipboardApplet::clipboardChanged()
{
    if ( menu ) {
	menu->setItemEnabled( 2, !qApp->clipboard()->text().isEmpty() );
    }
}

void ClipboardApplet::paintEvent( QPaintEvent* )
{
    QPainter p(this);
    p.drawPixmap( 0, (height()-clipboardPixmap.height())/2,
	clipboardPixmap );
}


