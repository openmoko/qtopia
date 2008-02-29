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

#ifndef QT_H
# include <qfeatures.h>
#endif // QT_H

#ifndef QT_NO_PROCESS
#ifndef QTOPIA_WIN32PROCESS_SUPPORT
#define QTOPIA_WIN32PROCESS_SUPPORT
#endif
#include "qapplication.h"
#include "qqueue.h"
#include "qtimer.h"

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include "process.h"

#ifndef QTOPIA_INTERNAL_FILEOPERATIONS
#define QTOPIA_INTERNAL_FILEOPERATIONS
#endif
#include <qtopia/global.h> // needed for Global::useUnicde()

/***********************************************************************
 *
 * ProcessPrivate
 *
 **********************************************************************/
class ProcessPrivate
{
public:
    ProcessPrivate( Process *proc )
    {
	stdinBufRead = 0;
	pipeStdin[0] = 0;
	pipeStdin[1] = 0;
	pipeStdout[0] = 0;
	pipeStdout[1] = 0;
	pipeStderr[0] = 0;
	pipeStderr[1] = 0;

	lookup = new QTimer( proc );
	qApp->connect( lookup, SIGNAL(timeout()),
		proc, SLOT(timeout()) );

	exitValuesCalculated = FALSE;
    }

    ~ProcessPrivate()
    {
	if( pipeStdin[1] != 0 )
	    CloseHandle( pipeStdin[1] );
	if( pipeStdout[0] != 0 )
	    CloseHandle( pipeStdout[0] );
	if( pipeStderr[0] != 0 )
	    CloseHandle( pipeStderr[0] );
    }

    HANDLE pipeStdin[2];
    HANDLE pipeStdout[2];
    HANDLE pipeStderr[2];
    QTimer *lookup;

    QByteArray bufStdin;
    QByteArray bufStdout;

    PROCESS_INFORMATION pid;
    uint stdinBufRead, stdoutBufWritten;

    bool exitValuesCalculated;
};

/***********************************************************************
 *
 * Process
 *
 **********************************************************************/
void Process::init()
{
    d = new ProcessPrivate( this );
    exitStat = 0;
    exitNormal = FALSE;
    d->stdinBufRead = 0;
    d->stdoutBufWritten = 0;
}

Process::~Process()
{
    delete d;
}

bool Process::exec( const QByteArray& in, QByteArray& out, QStringList *env )
{
#if defined(QT_QPROCESS_DEBUG)
    qDebug( "Process::exec()" );
#endif
    qWarning("Process not fully tested for WIN32");    
    
    d->bufStdin = in;
    d->bufStdout = out;

   // Open the pipes.  Make non-inheritable copies of input write and output
    // read handles to avoid non-closable handles.
    SECURITY_ATTRIBUTES secAtt = { sizeof( SECURITY_ATTRIBUTES ), NULL, TRUE };
    HANDLE tmpStdin, tmpStdout, tmpStderr;
    if ( !CreatePipe( &d->pipeStdin[0], &tmpStdin, &secAtt, 0 ) ) {
	return FALSE;
    }
    if ( !CreatePipe( &tmpStdout, &d->pipeStdout[1], &secAtt, 0 ) ) {
	return FALSE;
    }
    if ( !CreatePipe( &tmpStderr, &d->pipeStderr[1], &secAtt, 0 ) ) {
	return FALSE;
    }
    if ( !DuplicateHandle( GetCurrentProcess(), tmpStdin,
		GetCurrentProcess(), &d->pipeStdin[1],
		0, FALSE, DUPLICATE_SAME_ACCESS ) ) {
	return FALSE;
    }
    if ( !DuplicateHandle( GetCurrentProcess(), tmpStdout,
		GetCurrentProcess(), &d->pipeStdout[0],
		0, FALSE, DUPLICATE_SAME_ACCESS ) ) {
	return FALSE;
    }
    if ( !DuplicateHandle( GetCurrentProcess(), tmpStderr,
		GetCurrentProcess(), &d->pipeStderr[0],
		0, FALSE, DUPLICATE_SAME_ACCESS ) ) {
	return FALSE;
    }
    if ( !CloseHandle( tmpStdin ) ) {
	return FALSE;
    }
    if ( !CloseHandle( tmpStdout ) ) {
	return FALSE;
    }
    if ( !CloseHandle( tmpStderr ) ) {
	return FALSE;
    }

    // construct the arguments for CreateProcess()
    QString args;
    QStringList::Iterator it = env->begin();
    args = *it;
    ++it;
    for ( ; it != env->end(); ++it ) {
	args += QString( " \"" ) + (*it) + QString( "\"" );
    }

    // CreateProcess()
    bool success;
#if defined(UNICODE)
    if ( qt_winunicode ) {
	STARTUPINFO startupInfo = { sizeof( STARTUPINFO ), 0, 0, 0,
	    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
	    0, 0, 0,
	    STARTF_USESTDHANDLES,
	    0, 0, 0,
	    d->pipeStdin[0], d->pipeStdout[1], d->pipeStderr[1]
	};
	TCHAR *commandLine = (TCHAR*)qt_winTchar_new( args );
	success = CreateProcess( 0, commandLine,
		0, 0, TRUE, DETACHED_PROCESS, 0,
				 0L,
		&startupInfo, &d->pid );
	delete[] commandLine;
    } else
#endif
    {
	STARTUPINFOA startupInfo = { sizeof( STARTUPINFO ), 0, 0, 0,
	    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
	    0, 0, 0,
	    STARTF_USESTDHANDLES,
	    0, 0, 0,
	    d->pipeStdin[0], d->pipeStdout[1], d->pipeStderr[1]
	};
	success = CreateProcessA( 0, args.local8Bit().data(),
		0, 0, TRUE, DETACHED_PROCESS, 0,
				  0L,
		&startupInfo, &d->pid );
    }
    if  ( !success ) {
	return FALSE;
    }

    CloseHandle( d->pipeStdin[0] );
    CloseHandle( d->pipeStdout[1] );
    CloseHandle( d->pipeStderr[1] );

    // Start timer to poll our process for I/O
    d->lookup->start( 100 );

    // cleanup and return
    return TRUE;
}


#ifdef QTOPIA_WIN32PROCESS_SUPPORT
/*
  Use a timer for polling misc. stuff.
*/
#endif
void Process::timeout()
{    
    // try to write pending data to stdin
    DWORD written;
    if (d->stdinBufRead < d->bufStdin.size() ){
	//### revise 
	WriteFile(d->pipeStdin[1], d->bufStdin.data() + d->stdinBufRead, 
		  d->bufStdin.size() - d->stdinBufRead, &written, 0);
	d->stdinBufRead = d->bufStdin.size();
    }


    // get the number of bytes that are waiting to be read
    unsigned long i, r;
    char dummy;
    HANDLE dev = d->pipeStdout[0];
    if ( !PeekNamedPipe( dev, &dummy, 1, &r, &i, 0 ) ) {
	// ### revise  
	// ### is it worth to dig for the reason of the error?
	d->stdoutBufWritten = d->bufStdout.size();
    }else{
	d->bufStdout.resize(d->stdoutBufWritten + i);
	ReadFile(dev, d->bufStdout.data() + d->stdoutBufWritten, i, &r, 0);
	if ( r != i ) 
	    d->bufStdout.resize( d->bufStdout.size() - (i - r)) ;
        d->stdoutBufWritten = d->bufStdout.size();
    }

    // stop timer if process is not running
     if ( WaitForSingleObject( d->pid.hProcess, 0) == WAIT_OBJECT_0 ) {
	// compute the exit values
	if ( !d->exitValuesCalculated ) {
	    DWORD exitCode;
	    if ( GetExitCodeProcess( d->pid.hProcess, &exitCode ) ) {
		if ( exitCode != STILL_ACTIVE ) { // this should ever be true?
		    exitNormal = TRUE;
		    exitStat = exitCode;
		}
	    }
	    d->exitValuesCalculated = TRUE;
	}
	d->lookup->stop();
    }

}


#endif // QT_NO_PROCESS
