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

#include "qplatformdefs.h"
#include "qfile.h"
#include "qfiledefs_p.h"
#include "qdir.h"
#include "qt_mac.h"

#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>

extern const char* qt_fileerr_read;

const unsigned char * p_str(const char * c, int len=-1); //qglobal.cpp

bool qt_file_access( const QString& fn, int t )
{
    static FSSpec ret;
    const unsigned char *p = p_str(QFile::encodeName(QDir::convertSeparators(fn)));
    if(FSMakeFSSpec(0, 0, p, &ret) != noErr) {
	free(p);
	return FALSE;
    }
    free(p);
#if defined(Q_OS_UNIX)
    if ( fn.isEmpty() )
	return FALSE;
    return ACCESS( QFile::encodeName(fn), t ) == 0;
#else
    return TRUE;
#endif
}


bool QFile::remove( const QString &fileName )
{
    if ( fileName.isEmpty() ) {
#if defined(QT_CHECK_NULL)
	qWarning( "QFile::remove: Empty or null file name" );
#endif
	return FALSE;
    }
    return unlink( QFile::encodeName(QDir::convertSeparators(fileName)) ) == 0;
}

bool QFile::open( int m )
{
    if ( isOpen() ) {				// file already open
#if defined(QT_CHECK_STATE)
	qWarning( "QFile::open: File already open" );
#endif
	return FALSE;
    }
    if ( fn.isNull() ) {			// no file name defined
#if defined(QT_CHECK_NULL)
	qWarning( "QFile::open: No file name specified" );
#endif
	return FALSE;
    }
    init();					// reset params
    setMode( m );
    if ( !(isReadable() || isWritable()) ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QFile::open: File access not specified" );
#endif
	return FALSE;
    }
    bool ok = TRUE;
    struct stat st;
    if ( isRaw() ) {
	int oflags = O_RDONLY;
	if ( isReadable() && isWritable() )
	    oflags = O_RDWR;
	else if ( isWritable() )
	    oflags = O_WRONLY;
	if ( flags() & IO_Append ) {		// append to end of file?
	    if ( flags() & IO_Truncate )
		oflags |= (O_CREAT | O_TRUNC);
	    else
		oflags |= (O_APPEND | O_CREAT);
	    setFlags( flags() | IO_WriteOnly ); // append implies write
	} else if ( isWritable() ) {		// create/trunc if writable
	    if ( flags() & IO_Truncate )
		oflags |= (O_CREAT | O_TRUNC);
	    else
		oflags |= O_CREAT;
	}
#ifdef Q_OS_MAC9
	if ( isTranslated() )
	    oflags |= O_TEXT;
	else
	    oflags |= O_BINARY;
#endif
	fd = ::open( QFile::encodeName(QDir::convertSeparators(fn)), oflags, 0666 );

	if ( fd != -1 ) {			// open successful
	    ::fstat( fd, &st ); // get the stat for later usage
	} else {
	    ok = FALSE;
	}
    } else {					// buffered file I/O
	QCString perm;
	char perm2[4];
	bool try_create = FALSE;
	if ( flags() & IO_Append ) {		// append to end of file?
	    setFlags( flags() | IO_WriteOnly ); // append implies write
	    perm = isReadable() ? "a+" : "a";
	} else {
	    if ( isReadWrite() ) {
		if ( flags() & IO_Truncate ) {
		    perm = "w+";
		} else {
		    perm = "r+";
		    try_create = TRUE;		// try to create if not exists
		}
	    } else if ( isReadable() ) {
		perm = "r";
	    } else if ( isWritable() ) {
		perm = "w";
	    }
	}
	qstrcpy( perm2, perm );
#ifdef Q_OS_MAC9
	if ( isTranslated() )
	    strcat( perm2, "t" );
	else
	    strcat( perm2, "b" );
#endif
	for (;;) { // At most twice

	    fh = fopen( QFile::encodeName(QDir::convertSeparators(fn)), perm2 );

	    if ( !fh && try_create ) {
		perm2[0] = 'w';			// try "w+" instead of "r+"
		try_create = FALSE;
	    } else {
		break;
	    }
	}
	if ( fh ) {
	    ::fstat( fileno(fh), &st ); // get the stat for later usage
	} else {
	    ok = FALSE;
	}
    }
    if ( ok ) {
	setState( IO_Open );
	// on successful open the file stat was got; now test what type
	// of file we have
	if ( (st.st_mode & S_IFMT) != S_IFREG ) {
	    // non-seekable
	    setType( IO_Sequential );
	    length = INT_MAX;
	    ioIndex = (flags() & IO_Append) == 0 ? 0 : length;
	} else {
	    length = (Offset)st.st_size;
	    ioIndex = (flags() & IO_Append) == 0 ? 0 : length;
	    if ( !(flags()&IO_Truncate) && length == 0 && isReadable() ) {
		// try if you can read from it (if you can, it's a sequential
		// device; e.g. a file in the /proc filesystem)
		int c = getch();
		if ( c != -1 ) {
		    ungetch(c);
		    setType( IO_Sequential );
		    length = INT_MAX;
		}
		resetStatus();
	    }
	}
    } else {
	init();
	if ( errno == EMFILE )			// no more file handles/descrs
	    setStatus( IO_ResourceError );
	else
	    setStatus( IO_OpenError );
	setErrorStringErrno( errno );
    }
    return ok;
}

bool QFile::open( int m, FILE *f )
{
    if ( isOpen() ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QFile::open: File already open" );
#endif
	return FALSE;
    }
    init();
    setMode( m &~IO_Raw );
    setState( IO_Open );
    fh = f;
    ext_f = TRUE;
    struct stat st;
    ::fstat( fileno(fh), &st );
    ioIndex = (Offset)ftell( fh );
    if ( (st.st_mode & S_IFMT) != S_IFREG || f == stdin ) { //stdin is non seekable
	// non-seekable
	setType( IO_Sequential );
	length = INT_MAX;
    } else {
	length = (Offset)st.st_size;
	if ( !(flags()&IO_Truncate) && length == 0 && isReadable() ) {
	    // try if you can read from it (if you can, it's a sequential
	    // device; e.g. a file in the /proc filesystem)
	    int c = getch();
	    if ( c != -1 ) {
		ungetch(c);
		setType( IO_Sequential );
		length = INT_MAX;
	    }
	    resetStatus();
	}
    }
    return TRUE;
}


bool QFile::open( int m, int f )
{
    if ( isOpen() ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QFile::open: File already open" );
#endif
	return FALSE;
    }
    init();
    setMode( m |IO_Raw );
    setState( IO_Open );
    fd = f;
    ext_f = TRUE;
    struct stat st;
    ::fstat( fd, &st );
    ioIndex = (Offset)::lseek(fd, 0, SEEK_CUR);
    if ( (st.st_mode & S_IFMT) != S_IFREG || f == 0 ) { // stdin is not seekable...
	// non-seekable
	setType( IO_Sequential );
	length = INT_MAX;
    } else {
	length = (Offset)st.st_size;
	if ( length == 0 && isReadable() ) {
	    // try if you can read from it (if you can, it's a sequential
	    // device; e.g. a file in the /proc filesystem)
	    int c = getch();
	    if ( c != -1 ) {
		ungetch(c);
		setType( IO_Sequential );
		length = INT_MAX;
	    }
	    resetStatus();
	}
    }
    return TRUE;
}


QIODevice::Offset QFile::size() const
{
    struct stat st;
    int ret = 0;
    if ( isOpen() )
	ret = ::fstat( fh ? fileno(fh) : fd, &st );
    else
	ret = ::stat( QFile::encodeName(fn), &st );
    if ( ret == -1 )
	return 0;
    return st.st_size;
}


bool QFile::at( Offset pos )
{
    if ( !isOpen() ) {
#if defined(QT_CHECK_STATE)
	qWarning( "QFile::at: File is not open" );
#endif
	return FALSE;
    }
    bool ok;
    if ( isRaw() ) {
	pos = (Offset)::lseek( fd, pos, SEEK_SET );
	ok = (long int) pos != -1;		// ### fix this bad hack!
    } else {					// buffered file
	ok = ::fseek(fh, pos, SEEK_SET) == 0;
    }
    if ( ok )
	ioIndex = pos;
#if defined(QT_CHECK_RANGE)
    else
#if (QT_VERSION-0 >= 0x040000)
#error "QIODevice::Offset should be printed using %llu instead of %lu"
#elif defined(QT_LARGEFILE_SUPPORT) && defined(QT_ABI_64BITOFFSET)
	qWarning( "QFile::at: Cannot set file position %llu", pos );
#else
	qWarning( "QFile::at: Cannot set file position %lu", pos );
#endif

#endif
    return ok;
}


Q_LONG QFile::readBlock( char *p, Q_ULONG len )
{
    if ( !len ) // nothing to do
	return 0;

#if defined(QT_CHECK_NULL)
    if ( !p )
	qWarning( "QFile::readBlock: Null pointer error" );
#endif
#if defined(QT_CHECK_STATE)
    if ( !isOpen() ) {				// file not open
	qWarning( "QFile::readBlock: File not open" );
	return -1;
    }
    if ( !isReadable() ) {			// reading not permitted
	qWarning( "QFile::readBlock: Read operation not permitted" );
	return -1;
    }
#endif
    int nread = 0;					// number of bytes read
    if ( !ungetchBuffer.isEmpty() ) {
	// need to add these to the returned string.
	int l = ungetchBuffer.length();
	while( nread < l ) {
	    *p = ungetchBuffer[ l - nread - 1 ];
	    p++;
	    nread++;
	}
	ungetchBuffer.truncate( l - nread );
    }

    if ( nread < (int)len ) {
	if ( isRaw() ) {				// raw file
	    nread += ::read( fd, p, len-nread );
	    if ( len && nread <= 0 ) {
		nread = 0;
		setStatus(IO_ReadError);
		setErrorStringErrno( errno );
	    }
	} else {					// buffered file
	    nread += fread( p, 1, len-nread, fh );
	    if ( (uint)nread != len ) {
		if ( ferror( fh ) || nread==0 ) {
		    setStatus(IO_ReadError);
		    setErrorString( qt_fileerr_read );
		}
	    }
	}
    }
    ioIndex += nread;
    return nread;
}


Q_LONG QFile::writeBlock( const char *p, Q_ULONG len )
{
    if ( !len ) // nothing to do
	return 0;

#if defined(QT_CHECK_NULL)
    if ( p == 0 && len != 0 )
	qWarning( "QFile::writeBlock: Null pointer error" );
#endif
#if defined(QT_CHECK_STATE)
    if ( !isOpen() ) {				// file not open
	qWarning( "QFile::writeBlock: File not open" );
	return -1;
    }
    if ( !isWritable() ) {			// writing not permitted
	qWarning( "QFile::writeBlock: Write operation not permitted" );
	return -1;
    }
#endif
    int nwritten;				// number of bytes written
    if ( isRaw() )				// raw file
	nwritten = ::write( fd, (void *)p, len );
    else					// buffered file
	nwritten = fwrite( p, 1, len, fh );
    if ( nwritten != (int)len ) {		// write error
	if ( errno == ENOSPC )			// disk is full
	    setStatus( IO_ResourceError );
	else
	    setStatus( IO_WriteError );
	setErrorStringErrno( errno );
	if ( isRaw() )				// recalc file position
	    ioIndex = (Offset)::lseek( fd, 0, SEEK_CUR );
	else
	    ioIndex = ::fseek( fh, 0, SEEK_CUR );
    } else {
	ioIndex += nwritten;
    }
    if ( ioIndex > length )			// update file length
	length = ioIndex;
    return nwritten;
}

int QFile::handle() const
{
    if ( isOpen() )
	return fh ? fileno( fh ) : fd;
    return -1;
}

void QFile::close()
{
    bool ok = FALSE;
    if ( isOpen() ) {				// file is not open
	if ( fh ) {				// buffered file
	    if ( ext_f )
		ok = fflush( fh ) != -1;	// flush instead of closing
	    else
		ok = fclose( fh ) != -1;
	} else {				// raw file
	    if ( ext_f )
		ok = TRUE;			// cannot close
	    else
		ok = ::close( fd ) != -1;
	}
	init();					// restore internal state
    }
    if (!ok) {
	setStatus( IO_UnspecifiedError );
	setErrorStringErrno( errno );
    }
}
