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
#include <qtopia/qcopenvelope_qws.h>
#include <qtopia/qpeapplication.h>
#include "suspendmonitor.h"


#ifdef QTOPIA_MAX_SCREEN_DISABLE_TIME
#define QTOPIA_MIN_SCREEN_DISABLE_TIME ((int) 300)  // min 5 minutes before forced suspend kicks in
#endif


TempScreenSaverMonitor::TempScreenSaverMonitor(QObject *parent, const char *name)
    : QObject(parent, name)
{
    currentMode = QPEApplication::Enable;
    timerId = 0;
}

void TempScreenSaverMonitor::setTempMode(int mode, int pid)
{
    removeOld(pid);
    switch(mode) {
	case QPEApplication::Disable:
	    sStatus[0].append(pid);
	    break;
	case QPEApplication::DisableLightOff:
	    sStatus[1].append(pid);
	    break;
	case QPEApplication::DisableSuspend:
	    sStatus[2].append(pid);
	    break;
	case QPEApplication::Enable:
	    break;
	default:
	    qWarning("Unrecognized temp power setting.  Ignored");
	    return;
    }
    updateAll();
}

// Returns true if app had set a temp Mode earlier
bool TempScreenSaverMonitor::removeOld(int pid)
{
    QValueList<int>::Iterator it;
    for (int i = 0; i < 3; i++) {
	it = sStatus[i].find(pid);
	if ( it != sStatus[i].end() ) {
	    sStatus[i].remove( it );
	    return TRUE;
	}
    }
    return FALSE;
}

void TempScreenSaverMonitor::updateAll()
{
    int mode = QPEApplication::Enable;
    if ( sStatus[0].count() ) {
	mode = QPEApplication::Disable;
    } else if ( sStatus[1].count() ) {
	mode = QPEApplication::DisableLightOff;
    } else if ( sStatus[2].count() ) {
	mode = QPEApplication::DisableSuspend;
    } 

    if ( mode != currentMode ) {
#ifdef QTOPIA_MAX_SCREEN_DISABLE_TIME
	if ( currentMode == QPEApplication::Enable) {
	    int tid = timerValue();
	    if ( tid )
		timerId = startTimer( tid * 1000 );
	} else if ( mode == QPEApplication::Enable ) {
	    if ( timerId ) {
		killTimer(timerId);
		timerId = 0;
	    }
	}
#endif
	currentMode = mode;
	QCopEnvelope("QPE/System", "setScreenSaverMode(int)") << mode;
    }
}

void TempScreenSaverMonitor::applicationTerminated(int pid)
{
    if ( removeOld(pid) )
	updateAll();
}

int TempScreenSaverMonitor::timerValue()
{
    int tid = 0;
#ifdef QTOPIA_MAX_SCREEN_DISABLE_TIME
    tid = QTOPIA_MAX_SCREEN_DISABLE_TIME;

    char *env = getenv("QTOPIA_DISABLED_APM_TIMEOUT");
    if ( !env ) 
	return tid;

    QString strEnv = env;
    bool ok = FALSE;
    int envTime = strEnv.toInt(&ok);

    if ( ok ) {
	if ( envTime < 0 )
	    return 0;
	else if ( envTime <= QTOPIA_MIN_SCREEN_DISABLE_TIME )
	    return tid;
	else 
	    return envTime;
    }
#endif

    return tid;
}

void TempScreenSaverMonitor::timerEvent(QTimerEvent *t)
{
#ifdef QTOPIA_MAX_SCREEN_DISABLE_TIME
    if ( timerId && (t->timerId() == timerId) ) {
	
	/*  Clean up	*/
	killTimer(timerId);
	timerId = 0;
	currentMode = QPEApplication::Enable;
	QCopEnvelope("QPE/System", "setScreenSaverMode(int)") << currentMode;
	
	// signal starts on a merry-go-round, which ends up in Desktop::togglePower()
	emit forceSuspend();
	// if we have apm we are asleep at this point, next line will be executed when we
	// awake from suspend.
	if ( QFile::exists( "/proc/apm" ) ) {
	    QTime t;
	    t = t.addSecs( timerValue() );
	    QString str = tr("<qt>The running applications disabled the screen saver "
		             "for more than the allowed time (%1)."
			     "<br>The system was forced to suspend</qt>").arg( t.toString() );
	    QMessageBox::information(0, tr("Forced suspend"), str);
	}
	
	// Reset all requests.
	for (int i = 0; i < 3; i++)
	    sStatus[i].clear();

	updateAll();
    }
#else
    Q_UNUSED(t);
#endif
}

