/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#ifndef PERFTEST_H
#define PERFTEST_H

//
// For a performance testing build, each call to the PERFTEST macro
// prints out the passed message and the current time stamp.  For other
// builds it does nothing.
//

#ifdef QTOPIA_PERFTEST
#include <QTime>
#include <QDebug>
#define PERFTEST(message) \
    { \
        qDebug() << "PerfTest|" << message << "|" << qPrintable( QTime::currentTime().toString( "h:mm:ss.zzz" ) ); \
    }
#else
#define PERFTEST(message)
#endif

#endif   // PERFTEST_H
