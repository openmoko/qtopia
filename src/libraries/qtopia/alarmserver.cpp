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

#include "qlibrary.h"

#include <qdir.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qtextstream.h>


#include "qpeapplication.h"
#include "global.h"
#include "resource.h"

#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
#include "qcopenvelope_qws.h"
#endif
#include "alarmserver.h"
#include <qtopia/timeconversion.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <stdlib.h>
#ifndef Q_OS_WIN32
#include <unistd.h>
#else
#include <process.h>
#endif

struct timerEventItem {
    time_t UTCtime;
    QCString channel, message;
    int data;
    bool operator==( const timerEventItem &right ) const
    {
        return ( UTCtime == right.UTCtime
                 && channel == right.channel
                 && message == right.message
                 && data == right.data );
    }
};

class TimerReceiverObject : public QObject
{
  Q_OBJECT
public:
  TimerReceiverObject() { }
  ~TimerReceiverObject() { }
  void resetTimer();
  void setTimerEventItem();
  void deleteTimer();
protected:
  void timerEvent( QTimerEvent *te );
private:
    QString atfilename;
};

TimerReceiverObject *timerEventReceiver = NULL;
QList<timerEventItem> timerEventList;
timerEventItem *nearestTimerEvent = NULL;


// set the timer to go off on the next event in the list
void setNearestTimerEvent()
{
    nearestTimerEvent = NULL;
    QListIterator<timerEventItem> it( timerEventList );
    if ( *it )
	nearestTimerEvent = *it;
    for ( ; *it; ++it ) {
	if ( (*it)->UTCtime < nearestTimerEvent->UTCtime )
	    nearestTimerEvent = *it;
    }
    if (nearestTimerEvent)
	timerEventReceiver->resetTimer();
    else
	timerEventReceiver->deleteTimer();
}


//store current state to file
//Simple implementation. Should run on a timer.

static void saveState()
{
    QString savefilename = Global::applicationFileName( "AlarmServer", "saveFile" );
    if ( timerEventList.isEmpty() ) {
	unlink( savefilename );
	return;
    }

    QFile savefile(savefilename+".new");
    if ( savefile.open(IO_WriteOnly) ) {
	QDataStream ds( &savefile );

	//save
	QListIterator<timerEventItem> it( timerEventList );
	for ( ; *it; ++it ) {
	    ds << it.current()->UTCtime;
	    ds << it.current()->channel;
	    ds << it.current()->message;
	    ds << it.current()->data;
	}

	savefile.close();
	unlink( savefilename );
	QDir d; d.rename(savefilename+".new",savefilename);
    }
}

/*!
  Sets up the alarm server. Restoring to previous state (session management).
 */
void AlarmServer::initialize()
{
    // read autosave file and put events in timerEventList
    QString savefilename = Global::applicationFileName( "AlarmServer", "saveFile" );

    QFile savefile(savefilename);
    if ( savefile.open(IO_ReadOnly) ) {
	QDataStream ds( &savefile );
	while ( !ds.atEnd() ) {
	    timerEventItem *newTimerEventItem = new timerEventItem;
	    ds >> newTimerEventItem->UTCtime;
	    ds >> newTimerEventItem->channel;
	    ds >> newTimerEventItem->message;
	    ds >> newTimerEventItem->data;
	    timerEventList.append( newTimerEventItem );
	}
	savefile.close();
	if (!timerEventReceiver)
	    timerEventReceiver = new TimerReceiverObject;
	setNearestTimerEvent();
    }
}




static const char* atdir = "/var/spool/at/";

static bool triggerAtd( bool writeHWClock = FALSE )
{
    QFile trigger(QString(atdir) + "trigger"); // No tr
    if ( trigger.open(IO_WriteOnly|IO_Raw) ) {

	const char* data =
// ### removed this define as there's no way of updating the hardware clock if our atdaemon doesn't do it.
// Should probably revise this into a more general solution though.
	//custom atd only writes HW Clock if we write a 'W'
	    ( writeHWClock ) ? "W\n" :
	    data = "\n";
	int len = strlen(data);
	int total_written = trigger.writeBlock(data,len);
	if ( total_written != len ) {
	    QMessageBox::critical( 0, 0, qApp->translate( "AlarmServer",  "Out of Space" ),
				   qApp->translate( "AlarmServer", "<qt>Unable to schedule alarm. Free some memory and try again.</qt>" ) );
	    trigger.close();
	    QFile::remove( trigger.name() );
	    return FALSE;
	}
	return TRUE;
    }
    return FALSE;
}

void TimerReceiverObject::deleteTimer()
{
    if ( !atfilename.isEmpty() ) {
	unlink( atfilename );
	atfilename = QString::null;
	triggerAtd( FALSE );
    }
}

void TimerReceiverObject::resetTimer()
{
    const int maxsecs = 2147000;
    int total_written;
    QDateTime nearest = TimeConversion::fromUTC(nearestTimerEvent->UTCtime);
    QDateTime now = QDateTime::currentDateTime();
    if ( nearest < now )
	nearest = now;
    int secs = TimeConversion::secsTo( now, nearest );
    if ( secs > maxsecs ) {
	// too far for millisecond timing
	secs = maxsecs;
    }

    // System timer (needed so that we wake from deep sleep),
    // from the Epoch in seconds.
    //
    int at_secs = TimeConversion::toUTC(nearest);
    // qDebug("reset timer to %d seconds from Epoch",at_secs);
    QString fn = atdir + QString::number(at_secs) + "."
		 + QString::number(::getpid());
    if ( fn != atfilename ) {
	QFile atfile(fn+".new");
	if ( atfile.open(IO_WriteOnly|IO_Raw) ) {
	    // just wake up and delete the at file
	    QString cmd = "#!/bin/sh\nrm " + fn;
	    total_written = atfile.writeBlock(cmd.latin1(),cmd.length());
	    if ( total_written != int(cmd.length()) ) {
		QMessageBox::critical( 0, tr("Out of Space"),
				       tr("<qt>Unable to schedule alarm. "
					  "Please free up space and try again</qt>"));
		atfile.close();
		QFile::remove( atfile.name() );
		return;
	    }
	    atfile.close();
	    unlink( atfilename );
	    QDir d; d.rename(fn+".new",fn);
//### revise to suit WINNt?
#ifndef Q_OS_WIN32
	    chmod(fn.latin1(),0755);
#endif
	    atfilename = fn;
	    triggerAtd( FALSE );
	} else {
	    qWarning("Cannot open atd file %s",fn.latin1());
	}
    }
    // Qt timers (does the actual alarm)
    // from now in milliseconds
    //
    qDebug("AlarmServer waiting %d seconds",secs);

    static bool startup = TRUE;
    if (secs < 5 && startup)   // To delay the alarm when Qtopia first starts.
	secs = 5;
    startTimer( 1000 * secs + 500 );
    startup = FALSE;
}

void TimerReceiverObject::timerEvent( QTimerEvent * )
{
    bool needSave = FALSE;
    killTimers();
    if (nearestTimerEvent) {
        if ( nearestTimerEvent->UTCtime
	     <= TimeConversion::toUTC(QDateTime::currentDateTime()) ) {
#ifndef QT_NO_COP
	    QCopEnvelope e( nearestTimerEvent->channel,
			    nearestTimerEvent->message );
	    e << TimeConversion::fromUTC( nearestTimerEvent->UTCtime )
	      << nearestTimerEvent->data;
#endif
	    timerEventList.removeRef( nearestTimerEvent );
	    delete nearestTimerEvent;
	    nearestTimerEvent = 0;
	    needSave = TRUE;
	}
        setNearestTimerEvent();
    } else {
        resetTimer();
    }
    if ( needSave )
	saveState();
}

/*!
  \class AlarmServer alarmserver.h
  \brief The AlarmServer class allows alarms to be scheduled and unscheduled.

    Applications can schedule alarms with addAlarm() and can
    unschedule alarms with deleteAlarm(). When the time for an alarm
    to go off is reached the specified \link qcop.html QCop\endlink
    message is sent on the specified channel (optionally with
    additional data).

    Scheduling an alarm using this class is important (rather just using
    a QTimer) since the machine may be asleep and needs to get woken up using
    the Linux kernel which implements this at the kernel level to minimize
    battery usage while asleep.

    \ingroup qtopiaemb
    \sa QCopEnvelope
*/

/*!
  Schedules an alarm to go off at (or soon after) time \a when. When
  the alarm goes off, the \link qcop.html QCop\endlink \a message will
  be sent to \a channel, with \a data as a parameter.

  If this function is called with exactly the same data as a previous
  call the subsequent call is ignored, so there is only ever one alarm
  with a given set of parameters.

  \sa deleteAlarm()
*/
void AlarmServer::addAlarm ( QDateTime when, const QCString& channel,
			     const QCString& message, int data)
{
    if ( qApp->type() == QApplication::GuiServer ) {
	bool needSave = FALSE;
	// Here we are the server so either it has been directly called from
	// within the server or it has been sent to us from a client via QCop
	if (!timerEventReceiver)
	    timerEventReceiver = new TimerReceiverObject;

	timerEventItem *newTimerEventItem = new timerEventItem;
	newTimerEventItem->UTCtime = TimeConversion::toUTC( when );
	newTimerEventItem->channel = channel;
	newTimerEventItem->message = message;
	newTimerEventItem->data = data;
	// explore the case of already having the event in here...
	QListIterator<timerEventItem> it( timerEventList );
	for ( ; *it; ++it )
	    if ( *(*it) == *newTimerEventItem )
		return;
	// if we made it here, it is okay to add the item...
	timerEventList.append( newTimerEventItem );
	needSave = TRUE;
	// quicker than using setNearestTimerEvent()
	if ( nearestTimerEvent ) {
	    if (newTimerEventItem->UTCtime < nearestTimerEvent->UTCtime) {
	        nearestTimerEvent = newTimerEventItem;
	        timerEventReceiver->killTimers();
	        timerEventReceiver->resetTimer();
	    }
	} else {
	    nearestTimerEvent = newTimerEventItem;
	    timerEventReceiver->resetTimer();
	}
	if ( needSave )
	    saveState();
    } else {
#ifndef QT_NO_COP
        QCopEnvelope e( "QPE/System", "addAlarm(QDateTime,QCString,QCString,int)" );
        e << when << channel << message << data;
#endif
    }
}

/*!
  Deletes previously scheduled alarms which match \a when, \a channel,
  \a message, and \a data.

  Passing null values for \a when, \a channel, or for the \link
  qcop.html QCop\endlink \a message, acts as a wildcard meaning "any".
  Similarly, passing -1 for \a data indicates "any".

  If there is no matching alarm, nothing happens.

  \sa addAlarm()

*/
void AlarmServer::deleteAlarm (QDateTime when, const QCString& channel, const QCString& message, int data)
{
    if ( qApp->type() == QApplication::GuiServer) {
	bool needSave = FALSE;
	if ( timerEventReceiver ) {
	    timerEventReceiver->killTimers();

	    // iterate over the list of events
	    QListIterator<timerEventItem> it( timerEventList );
	    time_t deleteTime = TimeConversion::toUTC( when );
	    bool updatenearest = FALSE;
	    while ( *it ) {
		timerEventItem *event = *it;
		++it;
		// if its a match, delete it
		if ( ( event->UTCtime == deleteTime || when.isNull() )
		    && ( channel.isNull() || event->channel == channel )
		    && ( message.isNull() || event->message == message )
		    && ( data==-1 || event->data == data ) )
		{
		    // if it's first, then we need to update the timer
		    if ( event == nearestTimerEvent ) {
			updatenearest = TRUE;
			nearestTimerEvent = 0;
		    }
		    timerEventList.removeRef(event);
		    delete event;
		    needSave = TRUE;
		}
	    }

	    if ( updatenearest )
		setNearestTimerEvent();
	    else if ( nearestTimerEvent )
		timerEventReceiver->resetTimer();
	}
	if ( needSave )
	    saveState();
    } else {
#ifndef QT_NO_COP
        QCopEnvelope e( "QPE/System", "deleteAlarm(QDateTime,QCString,QCString,int)" );
        e << when << channel << message << data;
#endif
    }
}

#ifdef Q_WS_QWS
/*!
  Writes the system clock to the hardware clock.
*/
void Global::writeHWClock()
{
    if ( !triggerAtd( TRUE ) ) {
	// atd not running? set it ourselves
	system("/sbin/hwclock --systohc"); // ##### UTC?
    }
}
#endif

#include "alarmserver.moc"
