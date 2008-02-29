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
    QImage  img = Resource::loadImage("clipboard");
    img = img.smoothScale(AppLnk::smallIconSize(), AppLnk::smallIconSize());
    clipboardPixmap.convertFromImage(img);

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

qDebug("%d",i);
    if ( i == 0 )
	{ unicode='X'-'@'; scan=Key_X; } // Cut
    else if ( i == 1 )
	{ unicode='C'-'@'; scan=Key_C; } // Copy
    else if ( i == 2 )
	{ unicode='V'-'@'; scan=Key_V; } // Paste

    if ( scan ) {
qDebug("U%04d %d %d",unicode, scan, ControlButton);
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


