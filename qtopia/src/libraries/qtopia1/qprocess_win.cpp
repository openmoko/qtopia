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

//#include "qplatformdefs.h"
#include <qtopia/private/qpcglobal.h>
#ifndef QTOPIA_INTERNAL_FILEOPERATIONS
#define QTOPIA_INTERNAL_FILEOPERATIONS
#endif
#include <qtopia/global.h>
#define _OS_WIN32_
#define Q_OS_WIN32
#include "qprocess.h"

#ifndef QT_NO_PROCESS

#include "qapplication.h"
#include "qqueue.h"
#include "qtimer.h"
#include "qregexp.h"
#include "qt_windows.h"


/***********************************************************************
 *
 * QProcessPrivate
 *
 **********************************************************************/
class QProcessPrivate
{
public:
    QProcessPrivate( QProcess *proc )
    {
	stdinBufRead = 0;
	pipeStdin[0] = 0;
	pipeStdin[1] = 0;
	pipeStdout[0] = 0;
	pipeStdout[1] = 0;
	pipeStderr[0] = 0;
	pipeStderr[1] = 0;
	exitValuesCalculated = FALSE;

	lookup = new QTimer( proc );
	qApp->connect( lookup, SIGNAL(timeout()),
		proc, SLOT(timeout()) );

	pid = 0;
    }

    ~QProcessPrivate()
    {
	cleanup();
    }

    void cleanup()
    {
	while ( !stdinBuf.isEmpty() ) {
	    delete stdinBuf.dequeue();
	}
	if( pipeStdin[1] != 0 )
	    CloseHandle( pipeStdin[1] );
	if( pipeStdout[0] != 0 )
	    CloseHandle( pipeStdout[0] );
	if( pipeStderr[0] != 0 )
	    CloseHandle( pipeStderr[0] );
    }

    void reset()
    {
	cleanup();
	stdinBufRead = 0;
	pipeStdin[0] = 0;
	pipeStdin[1] = 0;
	pipeStdout[0] = 0;
	pipeStdout[1] = 0;
	pipeStderr[0] = 0;
	pipeStderr[1] = 0;
	exitValuesCalculated = FALSE;

	deletePid();
    }

    void deletePid()
    {
	delete pid;
	pid = 0;
    }

    void newPid()
    {
	delete pid;
	pid = new PROCESS_INFORMATION;
	memset( pid, 0, sizeof(PROCESS_INFORMATION) );
    }

    QByteArray bufStdout;
    QByteArray bufStderr;

    QQueue<QByteArray> stdinBuf;

    HANDLE pipeStdin[2];
    HANDLE pipeStdout[2];
    HANDLE pipeStderr[2];
    QTimer *lookup;

    PROCESS_INFORMATION *pid;
    uint stdinBufRead;

    bool exitValuesCalculated;
};


/***********************************************************************
 *
 * QProcess
 *
 **********************************************************************/
void QProcess::init()
{
    d = new QProcessPrivate( this );
    exitStat = 0;
    exitNormal = FALSE;
}

void QProcess::reset()
{
    d->reset();
    exitStat = 0;
    exitNormal = FALSE;
    d->bufStdout.resize( 0 );
    d->bufStderr.resize( 0 );
}

QByteArray* QProcess::bufStdout()
{
    if( d->pipeStdout[0] != 0 ) {
	socketRead( 1 );
    }
    return &d->bufStdout;
}

QByteArray* QProcess::bufStderr()
{
    if( d->pipeStderr[0] != 0 ) {
	socketRead( 2 );
    }
    return &d->bufStderr;
}

void QProcess::consumeBufStdout( int consume )
{
    uint n = d->bufStdout.size();
    if ( consume==-1 || (uint)consume >= n ) {
	d->bufStdout.resize( 0 );
    } else {
	QByteArray tmp( n - consume );
	memcpy( tmp.data(), d->bufStdout.data()+consume, n-consume );
	d->bufStdout = tmp;
    }
}

void QProcess::consumeBufStderr( int consume )
{
    uint n = d->bufStderr.size();
    if ( consume==-1 || (uint)consume >= n ) {
	d->bufStderr.resize( 0 );
    } else {
	QByteArray tmp( n - consume );
	memcpy( tmp.data(), d->bufStderr.data()+consume, n-consume );
	d->bufStderr = tmp;
    }
}


QProcess::~QProcess()
{
    delete d;
}

bool QProcess::start( QStringList *env )
{
#if defined(QT_QPROCESS_DEBUG)
    qDebug( "QProcess::start()" );
#endif
    reset();

    // Open the pipes.  Make non-inheritable copies of input write and output
    // read handles to avoid non-closable handles (this is done by the
    // DuplicateHandle() call).
    SECURITY_ATTRIBUTES secAtt = { sizeof( SECURITY_ATTRIBUTES ), NULL, TRUE };
#ifndef Q_OS_TEMP
    // I guess there is no stdin stdout and stderr on Q_OS_TEMP to dup
    // CreatePipe and DupilcateHandle aren't avaliable for Q_OS_TEMP
    HANDLE tmpStdin, tmpStdout, tmpStderr;
    if ( comms & Stdin ) {
	if ( !CreatePipe( &d->pipeStdin[0], &tmpStdin, &secAtt, 0 ) )
	    return FALSE;
	if ( !DuplicateHandle( GetCurrentProcess(), tmpStdin, GetCurrentProcess(), &d->pipeStdin[1], 0, FALSE, DUPLICATE_SAME_ACCESS ) )
	    return FALSE;
	if ( !CloseHandle( tmpStdin ) )
	    return FALSE;
    }
    if ( comms & Stdout ) {
	if ( !CreatePipe( &tmpStdout, &d->pipeStdout[1], &secAtt, 0 ) )
	    return FALSE;
	if ( !DuplicateHandle( GetCurrentProcess(), tmpStdout, GetCurrentProcess(), &d->pipeStdout[0], 0, FALSE, DUPLICATE_SAME_ACCESS ) )
	    return FALSE;
	if ( !CloseHandle( tmpStdout ) )
	    return FALSE;
    }
    if ( comms & Stderr ) {
	if ( !CreatePipe( &tmpStderr, &d->pipeStderr[1], &secAtt, 0 ) )
	    return FALSE;
	if ( !DuplicateHandle( GetCurrentProcess(), tmpStderr, GetCurrentProcess(), &d->pipeStderr[0], 0, FALSE, DUPLICATE_SAME_ACCESS ) )
	    return FALSE;
	if ( !CloseHandle( tmpStderr ) )
	    return FALSE;
    }
    if ( comms & DupStderr ) {
	CloseHandle( d->pipeStderr[1] );
	d->pipeStderr[1] = d->pipeStdout[1];
    }
#endif

    // construct the arguments for CreateProcess()
    QString args;
    QStringList::Iterator it = _arguments.begin();
    args = *it;
    ++it;
    for ( ; it != _arguments.end(); ++it ) {
	QString tmp = *it;
	// escape a single doublequote because the arguments will be parsed
	tmp.replace( QRegExp("\""), "\\\"" );
	// The argument must not end with a \ since this would be interpreted
	// as escaping the quote -- rather put the \ behind the quote: e.g.
	// rather use "foo"\ than "foo\"
	QString endQuote( "\"" );
	uint i = tmp.length();
	while ( i>=0 && tmp.at( i-1 ) == '\\' ) {
	    --i;
	    endQuote += "\\";
	}
	args += QString( " \"" ) + tmp.left( i ) + endQuote;
    }

    // CreateProcess()
    bool success;
    d->newPid();
#if defined(UNICODE)
#  ifndef Q_OS_TEMP
    if ( qt_winunicode) {
#  endif
	STARTUPINFO startupInfo = { sizeof( STARTUPINFO ), 0, 0, 0,
	    (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT,
	    0, 0, 0,
	    STARTF_USESTDHANDLES,
	    0, 0, 0,
	    d->pipeStdin[0], d->pipeStdout[1], d->pipeStderr[1]
	};
	TCHAR *commandLine = (TCHAR*)qt_winTchar_new( args );
	QByteArray envlist;
	if ( env != 0 ) {
	    int pos = 0;
	    // add PATH if necessary (for DLL loading)
	    char *path = getenv( "PATH" );
	    if ( env->grep( QRegExp("^PATH=",FALSE) ).isEmpty() && path ) {
		QString tmp = QString( "PATH=%1" ).arg( getenv( "PATH" ) );
		uint tmpSize = sizeof(TCHAR) * (tmp.length()+1);
		envlist.resize( envlist.size() + tmpSize );
		memcpy( envlist.data()+pos, qt_winTchar(tmp,TRUE), tmpSize );
		pos += tmpSize;
	    }
	    // add the user environment
	    for ( QStringList::Iterator it = env->begin(); it != env->end(); it++ ) {
		QString tmp = *it;
		uint tmpSize = sizeof(TCHAR) * (tmp.length()+1);
		envlist.resize( envlist.size() + tmpSize );
		memcpy( envlist.data()+pos, qt_winTchar(tmp,TRUE), tmpSize );
		pos += tmpSize;
	    }
	    // add the 2 terminating 0 (actually 4, just to be on the safe side)
	    envlist.resize( envlist.size()+4 );
	    envlist[pos++] = 0;
	    envlist[pos++] = 0;
	    envlist[pos++] = 0;
	    envlist[pos++] = 0;
	}
	success = CreateProcess( 0, commandLine,
		0, 0, TRUE, CREATE_NO_WINDOW
#ifndef Q_OS_TEMP
		| CREATE_UNICODE_ENVIRONMENT
#endif
		, env==0 ? 0 : envlist.data(),
		(TCHAR*)qt_winTchar(workingDir.absPath(),TRUE),
		&startupInfo, d->pid );
	delete[] commandLine;
#  ifndef Q_OS_TEMP
    } else
#  endif
#endif
#ifndef Q_OS_TEMP
    {
	STARTUPINFOA startupInfo = { sizeof( STARTUPINFOA ), 0, 0, 0,
	    (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT,
	    0, 0, 0,
	    STARTF_USESTDHANDLES,
	    0, 0, 0,
	    d->pipeStdin[0], d->pipeStdout[1], d->pipeStderr[1]
	};
	QByteArray envlist;
	if ( env != 0 ) {
	    int pos = 0;
	    // add PATH if necessary (for DLL loading)
	    char *path = getenv( "PATH" );
	    if ( env->grep( QRegExp("^PATH=",FALSE) ).isEmpty() && path ) {
		QCString tmp = QString( "PATH=%1" ).arg( getenv( "PATH" ) ).local8Bit();
		uint tmpSize = tmp.length() + 1;
		envlist.resize( envlist.size() + tmpSize );
		memcpy( envlist.data()+pos, tmp.data(), tmpSize );
		pos += tmpSize;
	    }
	    // add the user environment
	    for ( QStringList::Iterator it = env->begin(); it != env->end(); it++ ) {
		QCString tmp = (*it).local8Bit();
		uint tmpSize = tmp.length() + 1;
		envlist.resize( envlist.size() + tmpSize );
		memcpy( envlist.data()+pos, tmp.data(), tmpSize );
		pos += tmpSize;
	    }
	    // add the terminating 0 (actually 2, just to be on the safe side)
	    envlist.resize( envlist.size()+2 );
	    envlist[pos++] = 0;
	    envlist[pos++] = 0;
	}
	success = CreateProcessA( 0, args.local8Bit().data(),
		0, 0, TRUE, DETACHED_PROCESS,
		env==0 ? 0 : envlist.data(),
		(const char*)workingDir.absPath().local8Bit(),
		&startupInfo, d->pid );
    }
#endif
    if  ( !success ) {
	d->deletePid();
	return FALSE;
    }

#ifndef Q_OS_TEMP
    if ( (comms & Stdin) != 0 )
	CloseHandle( d->pipeStdin[0] );
    if ( (comms & Stdout) != 0 )
        CloseHandle( d->pipeStdout[1] );
    if ( (comms & Stderr) != 0 )
	CloseHandle( d->pipeStderr[1] );
#endif

    if ( ioRedirection || notifyOnExit ) {
	d->lookup->start( 100 );
    }

    // cleanup and return
    return TRUE;
}

BOOL CALLBACK qt_terminateApp( HWND hwnd, LPARAM procId )
{
  DWORD procId_win;
  GetWindowThreadProcessId( hwnd, &procId_win );
  if( procId_win == (DWORD)procId )
     PostMessage( hwnd, WM_CLOSE, 0, 0 );

  return TRUE;
}

void QProcess::tryTerminate() const
{
    EnumWindows( qt_terminateApp, (LPARAM)d->pid->dwProcessId );
}

void QProcess::kill() const
{
    if ( d->pid )
	TerminateProcess( d->pid->hProcess, 0xf291 );
}

bool QProcess::isRunning() const
{
    if ( !d->pid )
	return FALSE;

    if ( WaitForSingleObject( d->pid->hProcess, 0) == WAIT_OBJECT_0 ) {
	// there might be data to read
	QProcess *that = (QProcess*)this;
	that->socketRead( 1 ); // try stdout
	that->socketRead( 2 ); // try stderr
	// compute the exit values
	if ( !d->exitValuesCalculated ) {
	    DWORD exitCode;
	    if ( GetExitCodeProcess( d->pid->hProcess, &exitCode ) ) {
		if ( exitCode != STILL_ACTIVE ) { // this should ever be true?
		    QProcess *that = (QProcess*)this; // mutable 
		    that->exitNormal = exitCode != 0xf291;
		    that->exitStat = exitCode;
		}
	    }
	    d->exitValuesCalculated = TRUE;
	}
	d->deletePid();
	return FALSE;
    } else {
        return TRUE;
    }
}

void QProcess::writeToStdin( const QByteArray& buf )
{
    d->stdinBuf.enqueue( new QByteArray(buf) );
    socketWrite( 0 );
}

void QProcess::closeStdin( )
{
    if ( d->pipeStdin[1] != 0 ) {
	CloseHandle( d->pipeStdin[1] );
	d->pipeStdin[1] = 0;
    }
}

void QProcess::socketRead( int fd )
{
    // fd == 1: stdout, fd == 2: stderr
    HANDLE dev;
    if ( fd == 1 ) {
	dev = d->pipeStdout[0];
    } else if ( fd == 2 ) {
	dev = d->pipeStderr[0];
    } else {
	return;
    }
#ifndef Q_OS_TEMP
    // get the number of bytes that are waiting to be read
    unsigned long i, r;
    char dummy;
    if ( !PeekNamedPipe( dev, &dummy, 1, &r, &i, 0 ) ) {
	return; // ### is it worth to dig for the reason of the error?
    }
#else
    unsigned long i = 1000;
#endif
    if ( i > 0 ) {
	QByteArray *buffer;
	uint oldSize;
	if ( fd == 1 ) {
	    buffer = &d->bufStdout;
	} else {
	    buffer = &d->bufStderr;
	}

	oldSize = buffer->size();
	buffer->resize( oldSize + i );
	uint sz = readStddev( dev, buffer->data()+oldSize, i );
	if ( sz != i )
	    buffer->resize( oldSize + i );

	if ( sz == 0 )
	    return;
	if ( fd == 1 )
	    emit readyReadStdout();
	else
	    emit readyReadStderr();
    }
}

void QProcess::socketWrite( int fd )
{
    DWORD written;
    if ( d->stdinBuf.isEmpty() || !isRunning() ) {
	return;
    }
    if ( !WriteFile( d->pipeStdin[1],
		d->stdinBuf.head()->data() + d->stdinBufRead,
		d->stdinBuf.head()->size() - d->stdinBufRead,
		&written, 0 ) ) {
	return;
    }
    d->stdinBufRead += written;
    if ( d->stdinBufRead == d->stdinBuf.head()->size() ) {
	d->stdinBufRead = 0;
	delete d->stdinBuf.dequeue();
	if ( wroteToStdinConnected && d->stdinBuf.isEmpty() ) {
	    emit wroteToStdin();
	}
	socketWrite( fd );
	// start timer if there is still pending data
	if ( !d->stdinBuf.isEmpty() ) {
	    d->lookup->start( 100 );
	}
    }
}

void QProcess::flushStdin()
{
    socketWrite( 0 );
}

/*
  Use a timer for polling misc. stuff.
*/
void QProcess::timeout()
{
    // try to write pending data to stdin
    if ( !d->stdinBuf.isEmpty() ) {
	socketWrite( 0 );
	// stop timer if it is not needed any longer
	if ( d->stdinBuf.isEmpty() && !ioRedirection && !notifyOnExit )
	    d->lookup->stop();
    }

    if ( ioRedirection ) {
	socketRead( 1 ); // try stdout
	socketRead( 2 ); // try stderr
    }

    // stop timer if process is not running also emit processExited() signal
    if ( !isRunning() ) {
	d->lookup->stop();
	if ( notifyOnExit ) {
	    emit processExited();
	}
    }
}

/*
  read on the pipe
*/
uint QProcess::readStddev( HANDLE dev, char *buf, uint bytes )
{
    if ( bytes > 0 ) {
	ulong r;
	if ( ReadFile( dev, buf, bytes, &r, 0 ) )
	    return r;
    }
    return 0;
}

/*
  Used by connectNotify() and disconnectNotify() to change the value of
  ioRedirection (and related behaviour)
*/
void QProcess::setIoRedirection( bool value )
{
    ioRedirection = value;
    if ( !ioRedirection && !notifyOnExit )
	d->lookup->stop();
    if ( ioRedirection ) {
	if ( isRunning() )
	    d->lookup->start( 100 );
    }
}

/*
  Used by connectNotify() and disconnectNotify() to change the value of
  notifyOnExit (and related behaviour)
*/
void QProcess::setNotifyOnExit( bool value )
{
    notifyOnExit = value;
    if ( !ioRedirection && !notifyOnExit )
	d->lookup->stop();
    if ( notifyOnExit ) {
	if ( isRunning() )
	    d->lookup->start( 100 );
    }
}

/*
  Used by connectNotify() and disconnectNotify() to change the value of
  wroteToStdinConnected (and related behaviour)
*/
void QProcess::setWroteStdinConnected( bool value )
{
    wroteToStdinConnected = value;
}

QProcess::PID QProcess::processIdentifier()
{
    return d->pid;
}

int QProcess::priority() const
{
    return 0;
}

void QProcess::setPriority(int p)
{
}


#endif // QT_NO_PROCESS
