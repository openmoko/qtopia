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

#include <qpoint.h>
#include <qtimer.h>

#include "stylusnormalizer.h"

static const int FLUSHTIME = 100;

_StylusEvent::_StylusEvent( const QPoint& newPt )
    : _pt( newPt ),
      _t( QTime::currentTime() )
{
}

_StylusEvent::~_StylusEvent()
{
}

StylusNormalizer::StylusNormalizer( QWidget *parent, const char* name )
    : QWidget( parent, name ),
      _next( 0 ),
      bFirst( true )
{
    // initialize _ptList
    int i;
    for (i = 0; i < SAMPLES; i++ ) {
        _ptList[i].setPoint( -1, -1 );
    }
    _tExpire = new QTimer( this );
    QObject::connect( _tExpire, SIGNAL( timeout() ),
                      this, SLOT( slotAveragePoint() ) );
}

StylusNormalizer::~StylusNormalizer()
{
}

void StylusNormalizer::addEvent( const QPoint& pt )
{
    _ptList[_next].setPoint( pt );
    _ptList[_next++].setTime( QTime::currentTime() );
    if ( _next >= SAMPLES ) {
        _next = 0;
    }
    // make a single mouse click work
    if ( bFirst ) {
        slotAveragePoint();
        bFirst = false;
    }
}

void StylusNormalizer::slotAveragePoint( void )
{
    QPoint pt( 0, 0 );
    QTime tCurr = QTime::currentTime();
    int i,
        size;
    size = 0;
    for ( i = 0; i < SAMPLES; i++ ) {
        if ( ( (_ptList[i]).time().msecsTo( tCurr ) < FLUSHTIME ) &&
             ( _ptList[i].point() != QPoint( -1, -1 ) ) ) {
            pt += _ptList[i].point();
            size++;
        }
    }
    if ( size > 0 )
        emit signalNewPoint( pt /= size );
}

void StylusNormalizer::start( void )
{
    _tExpire->start( FLUSHTIME );
}

void StylusNormalizer::stop( void )
{
    _tExpire->stop();
    bFirst = true;
}

