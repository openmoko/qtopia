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

#include <qtopia/qpeglobal.h>

#ifndef Q_OS_WIN32
#include "qpedebug.h"
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

struct timeval qpe_debuglast;

void qpe_debugTime( const char *file, int line, const char *desc )
{
    struct timeval tv;
    gettimeofday( &tv, 0 );

    int tdiff = tv.tv_usec - qpe_debuglast.tv_usec;
    tdiff += (tv.tv_sec - qpe_debuglast.tv_sec) * 1000000;

    fprintf( stderr, "%s:%d Time: %ld.%06ld", file, line, tv.tv_sec, tv.tv_usec); // No tr

    static int pid = getpid();

    if ( qpe_debuglast.tv_sec )
	fprintf( stderr, " (pid %d delta %dus)", pid, tdiff ); // No tr

    if ( desc )
	fprintf( stderr, " (%s)", desc );

    fprintf( stderr, "\n" );

    qpe_debuglast = tv;
}

#endif

