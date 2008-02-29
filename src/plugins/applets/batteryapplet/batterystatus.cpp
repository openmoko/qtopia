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
#include "batterystatus.h"

#include <qtopia/power.h>

#include <qpainter.h>
#include <qpushbutton.h> 
#include <qdrawutil.h> 
#include <qtopia/qpeapplication.h>

static const int marg=10;

BatteryStatus::BatteryStatus( const PowerStatus *p, bool pop, QWidget *parent, WFlags f )
    : QFrame( parent, 0, f), ps(p), popup(pop)
{
    setBackgroundMode(PaletteButton);
    setCaption( tr("Battery Status") );
    percent = ps->batteryPercentRemaining();
}

BatteryStatus::~BatteryStatus()
{
}

QSize BatteryStatus::sizeHint() const
{
    QString text = statusText();
    QFontMetrics fm = fontMetrics();
    QRect r=fm.boundingRect( marg, 0, width()-marg*2, height(), AlignVCenter, text );
    return QSize(QMAX(QMIN(200, qApp->desktop()->width()),
	2*marg+r.width()),2*marg+60+r.height());
}

void BatteryStatus::updatePercent( int pc, bool doRepaint )
{
    percent = pc;
    if ( doRepaint )
	repaint(FALSE);
}

void BatteryStatus::drawSegment( QPainter *p, const QRect &r, const QColor &topgrad, const QColor &botgrad, const QColor &highlight, int hightlight_height )
{
    int j, h1, h2, s1, s2, v1, v2, ng = r.height(), hy = ng*30/100, hh = hightlight_height;
    topgrad.hsv( &h1, &s1, &v1 );
    botgrad.hsv( &h2, &s2, &v2 );
    for ( j = 0; j < hy-2; j++ ) {
	p->setPen( QColor( h1 + ((h2-h1)*j)/(ng-1), s1 + ((s2-s1)*j)/(ng-1),
		    v1 + ((v2-v1)*j)/(ng-1),  QColor::Hsv ) );
	p->drawLine( r.x(), r.top()+hy-2-j, r.x()+r.width(), r.top()+hy-2-j );
    }
    for ( j = 0; j < hh; j++ ) {
	p->setPen( highlight );
	p->drawLine( r.x(), r.top()+hy-2+j, r.x()+r.width(), r.top()+hy-2+j );
    }
    for ( j = 0; j < ng-hy-hh; j++ ) {
	p->setPen( QColor( h1 + ((h2-h1)*j)/(ng-1), s1 + ((s2-s1)*j)/(ng-1),
		    v1 + ((v2-v1)*j)/(ng-1),  QColor::Hsv ) );
	p->drawLine( r.x(), r.top()+hy+hh-2+j, r.x()+r.width(), r.top()+hy+hh-2+j );
    }
}

QString BatteryStatus::statusText() const
{
    QString text;
    if ( ps->batteryStatus() == PowerStatus::Charging ) {
	text = tr("Charging");
    } else if ( ps->batteryPercentAccurate() && percent > -1  ) {
	text.sprintf( tr("Percentage battery remaining") + ": %i%%", percent );
    } else {
	text = tr("Battery status: ");
	switch ( ps->batteryStatus() ) {
	    case PowerStatus::High:
		text += tr("Good");
		break;
	    case PowerStatus::Low:
		text += tr("Low");
		break;
	    case PowerStatus::VeryLow:
		text += tr("Very Low");
		break;
	    case PowerStatus::Critical:
		text += tr("Critical");
		break;
	    default: // NotPresent, etc.
		text += tr("Unknown");
	}
    }
    
    if (PowerStatusManager::APMEnabled()) {
        if ( ps->acStatus() == PowerStatus::Backup )
            text += "\n\n" + tr("On backup power");
        else if ( ps->acStatus() == PowerStatus::Online )
            text += "\n\n" + tr("Power on-line");
        else if ( ps->acStatus() == PowerStatus::Offline )
            text += "\n\n" + tr("External power disconnected");
    }
    if ( ps->batteryTimeRemaining() >= 0 ) {
	text += "\n\n" + QString().sprintf( tr("Battery time remaining") +
		": %im %02is", // No tr
	    ps->batteryTimeRemaining() / 60, ps->batteryTimeRemaining() % 60 );
    }

    return text;
}

void BatteryStatus::drawContents( QPainter *p )
{
    int ypos = height() / 20;
    QColor c;
    QColor darkc;
    QColor lightc;
    if ( ps->acStatus() == PowerStatus::Offline ) {
	c = blue.light(120);
	darkc = c.dark(280);
	lightc = c.light(145);
    } else if ( ps->acStatus() == PowerStatus::Online ) {
	c = green.dark(130);
	darkc = c.dark(200);
	lightc = c.light(220);
    } else {
	c = red;
	darkc = c.dark(280);
	lightc = c.light(140);
    }

    int length = percent * (width()-30) / 100;
    p->setPen( black );
    qDrawShadePanel( p,   9, ypos, width()-26, 39, colorGroup(), TRUE, 1, NULL);
    qDrawShadePanel( p, width()-18, ypos+7,  12, 24, colorGroup(), TRUE, 1, NULL);
    drawSegment( p, QRect( marg, ypos, length, 40 ), lightc, darkc, lightc.light(115), 6 );
    drawSegment( p, QRect( marg+1 + length, ypos, width()-30-length, 40 ), white.light(80), black, white.light(90), 6 );
    drawSegment( p, QRect( width()-marg-8, ypos+7, 10, 25 ), white.light(80), black, white.light(90), 2 );

    QString text = statusText();

    p->setPen( black );
    ypos += 60;
    p->fillRect( marg, ypos, width()-marg*2, height()-marg-ypos,
					palette().active().brush(QColorGroup::Button) );
    p->drawText( marg, ypos, width()-marg*2, height()-marg-ypos, AlignVCenter, text );
}

