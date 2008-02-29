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

#include "clock.h"

#include <qtopia/global.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>
#include <qtopia/resource.h>
#ifdef QWS
#include <qtopia/qcopenvelope_qws.h>
#endif

#include <qmainwindow.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qdatetime.h>
#include <qtimer.h>
#include <qpopupmenu.h>
#include <qpainter.h>
#include <qlabel.h>
#include <stdlib.h>


LauncherClock::LauncherClock( QWidget *parent ) : QLabel( parent )
{
    setFont( QFont( "Helvetica", 10, QFont::Normal ) );
    connect( qApp, SIGNAL( timeChanged() ), this, SLOT( updateTime() ) );
    TimeString::connectChange( this, SLOT( updateTime() ) );
    timerId = 0;
    updateTime();
    show();
}

void LauncherClock::mousePressEvent( QMouseEvent * )
{
    QPopupMenu *menu = new QPopupMenu(this);
    menu->insertItem(tr("Set time..."), 0);
    menu->insertSeparator();
    menu->insertItem(tr("Clock.."), 1);
    Config config( "Clock" );
    config.setGroup( "Daily Alarm" );
    bool alarmOn = config.readBoolEntry("Enabled", FALSE);
    QIconSet alarmIc(Resource::loadIconSet(alarmOn?"smallalarm":"smallalarm_off"));
    menu->insertItem(alarmIc, tr("Alarm..."), 2);
    QPoint curPos = mapToGlobal( QPoint(0,0) );
    QSize sh = menu->sizeHint();
    switch (menu->exec( curPos-QPoint((sh.width()-width())/2,sh.height()) )) {
	case 0:
	    Global::execute( "systemtime" );
	    break;
	case 1: {
	    //Global::execute( "clock" );
	    QCopEnvelope e("QPE/Application/clock", "showClock()" );
	    }
	    break;
	case 2: {
		QCopEnvelope e("QPE/Application/clock", "editDailyAlarm()" );
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
    setText( TimeString::localHM(tm) );

    if ( timerId )
	killTimer( timerId );
    QTime t = QTime::currentTime();
    int ms = (60 - t.second())*1000 - t.msec();
    timerId = startTimer( ms );
}

