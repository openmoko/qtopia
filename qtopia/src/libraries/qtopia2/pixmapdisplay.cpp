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

#include <qpixmap.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qlistbox.h>
#include <qstyle.h>
#include <qapplication.h>

#include "pixmapdisplay.h"

class PixmapDisplayPrivate
{
public:
    PixmapDisplayPrivate()
    {
    }
    QPixmap pix;
    QSize displaySize;
};

#if !(QT_VERSION >= 0x030000)
#define OLD_QSTYLE
#endif

const int MARGIN = 2;

//widget to display a pixmap to the user
PixmapDisplay::PixmapDisplay( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    init();
}

PixmapDisplay::PixmapDisplay( const QPixmap &pix, QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    init();
    setPixmap( pix );
}

PixmapDisplay::~PixmapDisplay()
{
    delete d;
}

void PixmapDisplay::init()
{
    d = new PixmapDisplayPrivate();
    setFocusPolicy( StrongFocus );
}

/* sets the display area for the pixmap to be size */
void PixmapDisplay::setDisplaySize( const QSize &size )
{
    d->displaySize = size;
    updateGeometry();
}

/* gets the display size for the pixmap */
QSize PixmapDisplay::displaySize() const
{
    return d->displaySize;
}

QSize PixmapDisplay::sizeHint() const
{
    const int bm = 
#ifdef OLD_QSTYLE
	style().buttonMargin();
#else
	QStyle::PM_ButtonMargin;
#endif

    const int border = bm*2 + (MARGIN * 2);
    int w = 0, h = 0;
    if( !d->displaySize.isEmpty() )
    {
	w = d->displaySize.width();
	h = d->displaySize.height();
    }
    else if( d->pix.isNull() )
    {
	w = 16;
	h = 16;
    }
    else
    {
	w = d->pix.width();
	h = d->pix.height();
    }
    return QSize( w+border, h+border ).expandedTo( QApplication::globalStrut() );
}

void PixmapDisplay::setPixmap( const QPixmap &pix )
{
    d->pix = pix;
    updateGeometry();
    repaint();
}

QPixmap PixmapDisplay::pixmap() const
{
    return d->pix;
}

void PixmapDisplay::paintEvent( QPaintEvent *e )
{
    QWidget::paintEvent( e );

    /*
    >= qt3 won't let me paint the widget like a button if it's not actually 
    a QButton, so just paint something static in that case -mrb 
    */
    QPainter p( this );

#ifdef OLD_QSTYLE
    style().drawButton( &p, 0, 0, width(), height(), colorGroup() );
#else
    p.setPen( colorGroup().foreground() );
    p.setBrush( colorGroup().background() );
    p.drawRect( 0, 0, width(), height() );
#endif

    if( !d->pix.isNull() )
    {
	int x = width()/2 - d->pix.width()/2;
	int y = height()/2 - d->pix.height()/2;
	p.drawPixmap( x, y, d->pix );
    }

    if( hasFocus() )
    {
#ifdef OLD_QSTYLE
	style().drawFocusRect( &p, QRect( 0, 0, width(), height() ), colorGroup() );
#else
	QPen pen( colorGroup().highlight(), MARGIN+1 );
	p.setPen( pen );
	p.setBrush( NoBrush );
	p.drawRect( 0, 0, width(), height() );
#endif
    }
}

QSizePolicy PixmapDisplay::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
}

void PixmapDisplay::keyPressEvent( QKeyEvent *e )
{
    QWidget::keyPressEvent( e );

    const int key = e->key();
    switch( key )
    {
#ifdef QTOPIA_PHONE
	case Key_Select:
#endif
	case Key_Enter:
	    emit clicked();
	    e->accept();
	default:
	    QWidget::keyPressEvent( e );
    }
}

void PixmapDisplay::mousePressEvent( QMouseEvent *e )
{
    int key = e->button();
    if( key & LeftButton )
	emit clicked();
    else
	QWidget::mousePressEvent( e );
}
