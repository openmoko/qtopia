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
#include "battery.h"
#include "batterystatus.h"

#include <qtopia/power.h>
#include <qtopia/config.h>

#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
#include <qtopia/qcopenvelope_qws.h>
#endif

#include <qpainter.h>
#include <qtimer.h>
#include <qapplication.h>

#include <qtopia/applnk.h>

BatteryMeter::BatteryMeter( QWidget *parent )
    : QWidget( parent ), charging(false)
{
    ps = new PowerStatus;
    startTimer( 10000 );
    setFixedWidth(QMAX(AppLnk::smallIconSize()*3/4,6));
    setFixedHeight(AppLnk::smallIconSize());
    chargeTimer = new QTimer( this );
    connect( chargeTimer, SIGNAL(timeout()), this, SLOT(chargeTimeout()) );
    timerEvent(0);
}

BatteryMeter::~BatteryMeter()
{
    delete (QWidget *) batteryView;
    delete ps;
}

QSize BatteryMeter::sizeHint() const
{
    return QSize(QMAX(AppLnk::smallIconSize()*3/4,6),AppLnk::smallIconSize());
}

void BatteryMeter::mousePressEvent( QMouseEvent *)
{
    if ( batteryView && batteryView->isVisible() ) {
	delete (QWidget *) batteryView;
	// batteryView becomes 0 because it's a QGuardedPtr
    } else {
	batteryView = new BatteryStatus( ps, TRUE, 0, WStyle_StaysOnTop | WType_Popup | WDestructiveClose );
	batteryView->setFrameStyle( QFrame::PopupPanel | QFrame::Raised );
	bvsz = QSize();
	updateBatteryViewGeometry();
	batteryView->raise();
	batteryView->show();
    }
}

bool BatteryMeter::updateBatteryViewGeometry()
{
    QSize sz=batteryView->sizeHint();
    if ( sz != bvsz ) {
	bvsz = sz;
	QRect r(batteryView->pos(),batteryView->sizeHint());
	QPoint curPos = mapToGlobal( rect().topLeft() );
	int lp = qApp->desktop()->width() - batteryView->sizeHint().width();
	r.moveTopLeft( QPoint(lp, curPos.y()-batteryView->sizeHint().height()-1) );
	batteryView->setGeometry(r);
	return TRUE;
    }
    return FALSE;
}

void BatteryMeter::timerEvent( QTimerEvent * )
{
    static bool first_update = TRUE;
    PowerStatus prev = *ps;
    
    *ps = PowerStatusManager::readStatus();
    if ( prev != *ps || first_update ) {
        if (first_update)
            first_update = FALSE;
	percent = ps->batteryPercentRemaining();
	if ( !charging && ps->batteryStatus() == PowerStatus::Charging && percent < 0 ) {
	    percent = 0;
	    charging = true;
	    chargeTimer->start( 500 );
	} else if ( charging && ps->batteryStatus() != PowerStatus::Charging ) {
	    charging = false;
	    chargeTimer->stop();
	}
	repaint(FALSE);
	if ( batteryView ) {
	    if ( updateBatteryViewGeometry() ) {
		batteryView->updatePercent( percent, FALSE );
		batteryView->update();
	    } else {
		batteryView->updatePercent( percent );
	    }
	}
    }
}

void BatteryMeter::chargeTimeout()
{
    percent += 20;
    if ( percent > 100 )
	percent = 0;

    repaint(FALSE);
    if ( batteryView )
	batteryView->updatePercent( percent );
}

void BatteryMeter::paintEvent( QPaintEvent* )
{
    QPainter p(this);

    QColor c;
    QColor darkc;
    QColor lightc;
    if ( ps->acStatus() == PowerStatus::Offline ) {
	c = blue.light(120);
	darkc = c.dark(120);
	lightc = c.light(140);
    } else if ( ps->acStatus() == PowerStatus::Online ) {
	c = green.dark(130);
	darkc = c.dark(120);
	lightc = c.light(180);
    } else {
	c = red;
	darkc = c.dark(120);
	lightc = c.light(160);
    }

    //
    // To simulate a 3-d battery, we use 4 bands of colour.  From left
    // to right, these are: medium, light, medium, dark.  To avoid
    // hardcoding values for band "width", figure everything out on the run.
    //
    int	batt_width;		    // width of each band
    int	batt_height;		    // battery height (not including terminal)
    int	used_height;		    // used amount of battery (scanlines)

    int	batt_yoffset;		    // top of terminal
    int batt_xoffset;		    // left edge of core

    int	band_width;		    // width of colour band

    int w = QMIN(height()/2, width());
    band_width = (w-2) / 4;
    if ( band_width < 1 )
	band_width = 1;
    
    batt_width = 4 * band_width + 2;	// +2 for 1 pixel border on both sides
    batt_height = height()-2;
    batt_xoffset = (width() - batt_width) / 2;
    batt_yoffset = (height() - batt_height) / 2;

    //
    // Battery border.  +1 to make space for the terminal at row 0.
    //
    p.setPen(QColor(80, 80, 80));
    p.drawRect(batt_xoffset, batt_yoffset+1, batt_width, batt_height);

    //
    // Draw terminal.  +1 to take into account the left border.
    //
    p.drawLine(batt_xoffset + band_width + 1, batt_yoffset,
	batt_xoffset + 3 * band_width, batt_yoffset);

    batt_height -= 2;	// -2 because we don't want to include border
    batt_yoffset += 2;  // +2 to account for border and terminal
    batt_xoffset++;

    //
    // 100 - percent, since percent is amount remaining, and we draw
    // reverse to this.
    //
    used_height = (100 - percent) * batt_height / 100;
    if (used_height < 0)
	used_height = 0;

    //
    // Drained section.
    //
    if (used_height != 0) {
	p.setPen(NoPen);
	p.setBrush(gray);
	p.drawRect(batt_xoffset, batt_yoffset, band_width, used_height);
	p.drawRect(batt_xoffset+2*band_width, batt_yoffset, band_width, used_height);

	p.setBrush(gray.light(130));
	p.drawRect(batt_xoffset+band_width, batt_yoffset, band_width, used_height);

	p.setBrush(gray.dark(120));
	p.drawRect(batt_xoffset+3*band_width, batt_yoffset, band_width, used_height);
    }

    //
    // Unused section.
    //
    if ( batt_height - used_height > 0 ) {
	int unused_offset = used_height + batt_yoffset;
	int unused_height = batt_height - used_height;
	p.setPen(NoPen);
	p.setBrush(c);
	p.drawRect(batt_xoffset, unused_offset, band_width, unused_height);
	p.drawRect(batt_xoffset+2*band_width, unused_offset, band_width, unused_height);

	p.setBrush(lightc);
	p.drawRect(batt_xoffset+band_width, unused_offset, band_width, unused_height);

	p.setBrush(darkc);
	p.drawRect(batt_xoffset+3*band_width, unused_offset, band_width, unused_height);
    }
}

