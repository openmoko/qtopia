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


#include "stabmon.h"

#ifdef QWS
#include <qtopia/qcopenvelope_qws.h>
#endif

#include <qfile.h>
#include <qcstring.h>

#include <sys/stat.h>
#if defined(Q_OS_LINUX) || defined(_OS_LINUX_)
#include <unistd.h>
#endif
#include <stdlib.h>

SysFileMonitor::SysFileMonitor(QObject* parent) :
    QObject(parent)
{
    startTimer(2000);
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

