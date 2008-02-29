/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


#include "stabmon.h"

#include <qtopiaipcenvelope.h>
#include "qtopiaserverapplication.h"
#include <QSettings>
#include <QFile>
#include <QtopiaChannel>
#include <QTextStream>
#include <QDebug>

#include <sys/stat.h>
#ifdef Q_OS_LINUX
#include <unistd.h>
#endif
#include <stdlib.h>

/*!
  \class SysFileMonitor
  \ingroup QtopiaServer::Task
  \brief The SysFileMonitor class polls Linux's mount and PCMCIA status for
         changes.

  Qtopia requires notification, by the way of a QCop message, whenever the
  attached PCMCIA devices or the mounted filesystems change.  It uses this
  notification to rescan the environment appropriately.

  Sometimes it is not possible, or the Linux system has not been configured, in
  such a way that these notifications are only sent on an actual change.  In
  these scenarios it may be necessary to regularly poll files to determine when
  things have changes.

  The SysFileMonitor task polls the following files and sends the specified
  QCop messages when they change:

  \table
  \header \o File \o QCop Channel \o QCop Message
  \row \o /var/run/stab \o \c {QPE/Card} \o \c {stabChanged()}
  \row \o /var/state/pcmcia/stab \o \c {QPE/Card} \o \c {stabChanged()}
  \row \o /var/lib/pcmcia/stab \o \c {QPE/Card} \o \c {stabChanged()}
  \row \o /proc/mounts \o \c {QPE/Card} \o \c {mtabChanged()}
  \endtable

  It is generally more efficient and responsive if these noticiations are sent
  via the QCop command line tool from mount scripts within Linux, in which case
  the SysFileMonitor task is not needed.

  The SysFileMonitor task responds to configuration through the
  \c {Trolltech/qpe} settings file.  The following keys have meaning:

  \table
  \header \o Key \o Description \o Default
  \row \o \c {SysFileMonitor/Enabled} \o Enables or disables the monitor.  Whether or not the monitor is enabled can be queried through the \c {/System/Tasks/SysFileMonitor/InUse} value space key. \o true
  \row \o \c {SysFileMonitor/PollRate} \o The frequency, in milliseconds, with which the monitor polls. \o 2000ms
  \endtable

  The SysFileMonitor class provides the \c {SysFileMonitor} task.
 */

/*!
  Create the SysFileMonitor class with the specified \a parent.
 */
SysFileMonitor::SysFileMonitor(QObject* parent)
: QObject(parent), tid(0)
{
    QSettings cfg("Trolltech","qpe");
    cfg.beginGroup("SysFileMonitor");
    if (!cfg.value("Enabled", true).toBool()) {
        QtopiaServerApplication::taskValueSpaceSetAttribute("SysFileMonitor",
                                                            "InUse",
                                                            "No");
        return;
    } else {
        QtopiaServerApplication::taskValueSpaceSetAttribute("SysFileMonitor",
                                                            "InUse",
                                                            "Yes");
    }

    rate = cfg.value("PollRate", 2000).toInt();
    if(rate < 100) rate = 2000;

    tid = startTimer(rate);
}

/*!
  Temporarily suspend polling.
 */
void SysFileMonitor::suspend()
{
    if (tid){
        killTimer(tid);
        tid = 0;
    }
}

/*!
  Resume polling following a suspend.  If the monitor is not suspended, this
  method has no effect.
 */
void SysFileMonitor::restart()
{
    if (tid)
        return;
    tid = startTimer(rate);
}

const char * stab0 = "/var/run/stab";
const char * stab1 = "/var/state/pcmcia/stab";
const char * stab2 = "/var/lib/pcmcia/stab";

/*!  \internal */
void SysFileMonitor::timerEvent(QTimerEvent*)
{
    struct stat s;

    static const char * tab [] = {
        stab0,
        stab1,
        stab2
    };
    static const int nstab = sizeof(tab)/sizeof(const char *);
    static int last[nstab];

    bool ch = false;
    for ( int i=0; i<nstab; i++ ) {
        if ( ::stat(tab[i], &s)==0 && (long)s.st_mtime != last[i] ) {
            last[i] = (long)s.st_mtime;
            ch=true;
        }
        if ( ch ) {
            QtopiaIpcEnvelope("QPE/Card", "stabChanged()" );
            break;
        }
    }

    // st_size is no use, it's 0 for /proc/mounts too. Read it all.
    static int mtabSize = 0;
    QFile f( "/proc/mounts" );
    if ( f.open(QIODevice::ReadOnly | QIODevice::Text) ) {
        QTextStream in(&f);
        QString s = in.readAll();
        if ( (int)s.length() != mtabSize ) {
            mtabSize = (int)s.length();
            // The QtopiaSqlPrivate singleton in the server needs to deal with
            // this message before any non-server processes see it. Since there
            // is no "synchronous QCop", we send a message that nobody else is
            // expecting. QtopiaSqlPrivate will send the mtabChanged() message.
            QtopiaIpcEnvelope("QPE/Card", "mtabChanged_qtopiasql()" );
        }
    }
}

QTOPIA_TASK(SysFileMonitor, SysFileMonitor);
