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

#define QTOPIA_INTERNAL_FILEOPERATIONS

#include <qtopia/global.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

QString quickExecPipe;

int quickexecv( const char *path, const char *argv[] )
{
    if (quickExecPipe.isEmpty())
	quickExecPipe = Global::tempDir() + ".quickexec";

    int fd = ::open( quickExecPipe.local8Bit().data(), O_WRONLY );
    if ( fd == -1 ) {
	perror( "quickexec pipe" );
	return -1;
    }
    write( fd, path, strlen( path )+1 );
    const char **s = argv;
    while( *s ) {
	write( fd, *s, strlen( *s )+1 );
	++s;
    }
    close(fd);
    return 0;
}

int quickexec( const char *path, const char *, ...)
{
    if (quickExecPipe.isEmpty())
	quickExecPipe = Global::tempDir() + ".quickexec";

    int fd = ::open( quickExecPipe.local8Bit().data(), O_WRONLY );
    if ( fd == -1 ) {
	perror( "quickexec pipe" );
	return -1;
    }
    const char** s = &path;
    do {
	write( fd, *s, strlen( *s )+1 );
    } while ( *(++s) );

    close( fd );

    return 0;
}
