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

#include "clock.h"

#include <qpe/global.h>
#include <qpe/qpeapplication.h>
#include <qpe/config.h>
#include <qpe/resource.h>
#ifdef QWS
#include <qpe/qcopenvelope_qws.h>
#endif

#include <qmainwindow.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qdatetime.h>
#include <qtimer.h>
#include <qpopupmenu.h>
#include <qlabel.h>
#include <stdlib.h>


LauncherClock::LauncherClock( QWidget *parent ) : QLabel( parent )
{
    setFont( QFont( "Helvetica", 10, QFont::Normal ) );
    connect( qApp, SIGNAL( timeChanged() ), this, SLOT( updateTime( ) ) );
    connect( qApp, SIGNAL( clockChanged( bool ) ),
	     this, SLOT( slotClockChanged( bool ) ) );
    Config config( "qpe" );
    config.setGroup( "Time" );
    ampmFormat = config.readBoolEntry( "AMPM", TRUE );
    timerId = 0;
    updateTime();
    show();
}

void LauncherClock::mousePressEvent( QMouseEvent * )
{
    QPopupMenu *menu = new QPopupMenu(this);
    menu->insertItem(tr("Set time"), 0);
    menu->insertSeparator();
    menu->insertItem(tr("Clock"), 1);
    Config config( "Clock" );
    config.setGroup( "Daily Alarm" );
    bool alarmOn = config.readBoolEntry("Enabled", FALSE);
    if ( alarmOn )
	menu->insertItem(Resource::loadPixmap("smallalarm"), tr("Alarm"), 2);
    else
	menu->insertItem(tr("Alarm"), 2);
    menu->setItemChecked( 2, alarmOn );
    QPoint curPos = mapToGlobal( QPoint(0,0) );
    QSize sh = menu->sizeHint();
    switch (menu->exec( curPos-QPoint((sh.width()-width())/2,sh.height()) )) {
	case 0:
	    Global::execute( "systemtime" );
	    break;
	case 1:
	    Global::execute( "clock" );
	    break;
	case 2:
	    {
		QCopEnvelope e("QPE/Application/clock", "setDailyEnabled(int)" );
		e << (menu->isItemChecked(2) ? 0 : 1);
	    }
	    break;
	default:
	    break;
    }
    delete menu;
}

void LauncherClock::timerEvent( QTimerEvent *e )
{
    if ( e->timerId() == timerId ) {
	updateTime();
    } else {
	QLabel::timerEvent( e );
    }
}

void LauncherClock::updateTime( void )
{
    QTime tm = QDateTime::currentDateTime().time();
    QString s;
    if( ampmFormat ) {
	int hour = tm.hour(); 
	if (hour == 0)
	    hour = 12;
	if (hour > 12)
	    hour -= 12;
	s.sprintf( "%2d%c%02d %s", hour, ':', tm.minute(), (tm.hour() >= 12) ? "PM" : "AM" );
    } else
	s.sprintf( "%2d%c%02d", tm.hour(), ':', tm.minute() );
    setText( s );

    if ( timerId )
	killTimer( timerId );
    QTime t = QTime::currentTime();
    int ms = (60 - t.second())*1000 - t.msec();
    timerId = startTimer( ms );
}

void LauncherClock::slotClockChanged( bool pm )
{
    ampmFormat = pm;
    updateTime();
}
