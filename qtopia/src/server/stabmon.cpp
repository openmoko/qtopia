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


#include "stabmon.h"

#ifdef QWS
#include <qtopia/qcopenvelope_qws.h>
#endif

#include <qfile.h>
#include <qcopchannel_qws.h>

#include <sys/stat.h>
#if defined(Q_OS_LINUX) || defined(_OS_LINUX_)
#include <unistd.h>
#endif
#include <stdlib.h>

SysFileMonitor::SysFileMonitor(QObject* parent) :
    QObject(parent)
{
    startTimer(2000);
#ifndef QT_NO_COP
    QCopChannel *chan = new QCopChannel( "QPE/Stabmon", this );
    connect( chan, SIGNAL(received(const QCString&,const QByteArray&)),
	     this, SLOT(qcopMessage(const QCString&,const QByteArray&)) );
#endif
}

void SysFileMonitor::qcopMessage( const QCString &message, const QByteArray & )
{
    qDebug( "SysFileMonitor::qcopMessage %s", message.data() );
    if ( message == "suspendMonitor()" ) {
	killTimers();
    } else if ( message == "restartMonitor()" ) {
	killTimers(); // just in case
	startTimer( 2000 );
    }
}

const char * stab0 = "/var/run/stab";
const char * stab1 = "/var/state/pcmcia/stab";
const char * stab2 = "/var/lib/pcmcia/stab";

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

    bool ch = FALSE;
    for ( int i=0; i<nstab; i++ ) {
	if ( ::stat(tab[i], &s)==0 && (long)s.st_mtime != last[i] ) {
	    last[i] = (long)s.st_mtime;
	    ch=TRUE;
	}
	if ( ch ) {
#ifndef QT_NO_COP
	    QCopEnvelope("QPE/Card", "stabChanged()" );
#endif
	    break;
	}
    }
    
    // st_size is no use, it's 0 for /proc/mounts too. Read it all.
    static int mtabSize = 0;
    QFile f( "/etc/mtab" );
    if ( f.open(IO_ReadOnly) ) {
#if 0
	// readAll does not work correctly on sequential devices (as eg. /proc files)
	QByteArray ba = f.readAll();
	if ( (int)ba.size() != mtabSize ) {
	    mtabSize = (int)ba.size();
#ifndef QT_NO_COP
	    QCopEnvelope("QPE/Card", "mtabChanged()" );
#endif
	}
#else
	QString s;
	while( !f.atEnd() ) {
	    QString tmp;
	    f.readLine( tmp, 1024 );
	    s += tmp;
	}
	if ( (int)s.length() != mtabSize ) {
	    mtabSize = (int)s.length();
#ifndef QT_NO_COP
	    QCopEnvelope("QPE/Card", "mtabChanged()" );
#endif
	}
#endif	    
    }
}

