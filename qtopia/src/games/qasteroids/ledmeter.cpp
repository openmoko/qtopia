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

/*********************************************************************
** Asteroids(R) is a registered trademark of Atari Corporation.  Please
** note that the Asteroids(R) games included in Qtopia was not developed
** by nor is endorsed by Atari Corporation. Trolltech respectfully
** acknowledges Atari Corporation's ownership of the trademark."
**********************************************************************/

/*
 * KAsteroids - Copyright (c) Martin R. Jones 1997
 *
 * Part of the KDE project
 */

#include <qpainter.h>
#include "ledmeter.h"

KALedMeter::KALedMeter( QWidget *parent ) : QFrame( parent )
{
    mCRanges.setAutoDelete( TRUE );
    mRange = 100;
    mCount = 20;
    mCurrentCount = 0;
    mValue = 0;
    setMinimumWidth( mCount * 2 + frameWidth() );
}

void KALedMeter::setRange( int r )
{
    mRange = r;
    if ( mRange < 1 )
        mRange = 1;
    setValue( mValue );
    update();
}

void KALedMeter::setCount( int c )
{
    mCount = c;
    if ( mCount < 1 )
        mCount = 1;
    setMinimumWidth( mCount * 2 + frameWidth() );
    calcColorRanges();
    setValue( mValue );
    update();
}

void KALedMeter::setValue( int v )
{
    mValue = v;
    if ( mValue > mRange )
        mValue = mRange;
    else if ( mValue < 0 )
        mValue = 0;
    int c = ( mValue + mRange / mCount - 1 ) * mCount / mRange;
    if ( c != mCurrentCount )
    {
        mCurrentCount = c;
        update();
    }
}

void KALedMeter::addColorRange( int pc, const QColor &c )
{
    ColorRange *cr = new ColorRange;
    cr->mPc = pc;
    cr->mColor = c;
    mCRanges.append( cr );
    calcColorRanges();
}

void KALedMeter::resizeEvent( QResizeEvent *e )
{
    QFrame::resizeEvent( e );
    int w = ( width() - frameWidth() - 2 ) / mCount * mCount;
    w += frameWidth() + 2;
    setFrameRect( QRect( 0, 0, w, height() ) );
}

void KALedMeter::drawContents( QPainter *p )
{
    QRect b = contentsRect();

    unsigned cidx = 0;
    int ncol = mCount;
    QColor col = colorGroup().foreground();
   
    if ( !mCRanges.isEmpty() )
    {
        col = mCRanges.at( cidx )->mColor;
        ncol = mCRanges.at( cidx )->mValue;
    }
    p->setBrush( col );
    p->setPen( col );

    int lw = b.width() / mCount;
    int lx = b.left() + 1;
    for ( int i = 0; i < mCurrentCount; i++, lx += lw )
    {
        if ( i > ncol )
        {
            if ( ++cidx < mCRanges.count() )
            {
                col = mCRanges.at( cidx )->mColor;
                ncol = mCRanges.at( cidx )->mValue;
                p->setBrush( col );
                p->setPen( col );
            }
        }

        p->drawRect( lx, b.top() + 1, lw - 1, b.height() - 2 );
    }
}

void KALedMeter::calcColorRanges()
{
    int prev = 0;
    ColorRange *cr;
    for ( cr = mCRanges.first(); cr; cr = mCRanges.next() )
    {
        cr->mValue = prev + cr->mPc * mCount / 100;
        prev = cr->mValue;
    }
}

