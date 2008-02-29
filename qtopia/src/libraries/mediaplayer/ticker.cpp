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
#include <qpainter.h>
#include "imageutil.h"
#include "ticker.h"


/*!
  \class Ticker ticker.h
  \brief This Ticker class is specific to the mediaplayer apps.

    It is currently private to those apps, and its API,
    and therefore binary compatibilty may change in later
    versions.

    This class displays text similar to a non-editable
    QLineEdit but will scroll the text if it does not fit.
*/


Ticker::Ticker( QWidget* parent ) : QFrame( parent )
{
    setFrameStyle( WinPanel | Sunken );
    scrollText = tr( "No Song" );
    pos = 0;
}


Ticker::~Ticker()
{
}


void Ticker::setText( const QString& text )
{
    scrollText = text;

    int pixelLen = fontMetrics().width( text );
    QPixmap pm( pixelLen, contentsRect().height() );
    pm.fill( QColor( 167, 212, 167 ) );
    QPainter pmp( &pm );
    pmp.setPen( Qt::black );
    pmp.drawText( 0, 0, pixelLen, contentsRect().height(), AlignVCenter, scrollText );
    pmp.end();
    scrollTextPixmap = pm;

    killTimers();
    if ( pixelLen > contentsRect().width() )
	startTimer( 50 );
    else
	pos = (width() - pixelLen) / 2; // Center it only if not scrolling, else don't move it

    // repaint
    repaint( false );
}


void Ticker::resizeEvent( QResizeEvent * )
{
    setText( scrollText );
}


void Ticker::timerEvent( QTimerEvent * )
{
    pos = ( pos <= 0 ) ? scrollTextPixmap.width() : pos - 1;
    repaint( false );
}


void Ticker::drawContents( QPainter *p )
{
    int pixelLen = scrollTextPixmap.width(); 
    if ( pixelLen > contentsRect().width() ) { // Scrolling
	p->drawPixmap( contentsRect().x()+pos-pixelLen, contentsRect().y(), scrollTextPixmap );
    } else {
	QBrush brush( QColor( 167, 212, 167 ) );
	p->fillRect( contentsRect().x(), contentsRect().y(),
			contentsRect().x()+pos, contentsRect().height(), brush );
	p->fillRect( contentsRect().x()+pos+pixelLen, contentsRect().y(),
			contentsRect().width()-(pos+pixelLen), contentsRect().height(), brush );
    }
    p->drawPixmap( contentsRect().x()+pos, contentsRect().y(), scrollTextPixmap );
}

