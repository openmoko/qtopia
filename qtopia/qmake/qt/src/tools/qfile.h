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

#ifndef QFILE_H
#define QFILE_H

#ifndef QT_H
#include "qiodevice.h"
#include "qstring.h"
#include <stdio.h>
#endif // QT_H

class QDir;
class QFilePrivate;

class Q_EXPORT QFile : public QIODevice			// file I/O device class
{
public:
    QFile();
    QFile( const QString &name );
   ~QFile();

    QString	name()	const;
    void	setName( const QString &name );

    typedef QCString (*EncoderFn)( const QString &fileName );
    typedef QString (*DecoderFn)( const QCString &localfileName );
    static QCString encodeName( const QString &fileName );
    static QString decodeName( const QCString &localFileName );
    static void setEncodingFunction( EncoderFn );
    static void setDecodingFunction( DecoderFn );

    bool	exists()   const;
    static bool exists( const QString &fileName );

    bool	remove();
    static bool remove( const QString &fileName );

    bool	open( int );
    bool	open( int, FILE * );
    bool	open( int, int );
    void	close();
    void	flush();

    Offset	size() const;
    Offset	at() const;
    bool	at( Offset );
    bool	atEnd() const;

    Q_LONG	readBlock( char *data, Q_ULONG len );
    Q_LONG	writeBlock( const char *data, Q_ULONG len );
    Q_LONG	writeBlock( const QByteArray& data )
		      { return QIODevice::writeBlock(data); }
    Q_LONG	readLine( char *data, Q_ULONG maxlen );
    Q_LONG	readLine( QString &, Q_ULONG maxlen );

    int		getch();
    int		putch( int );
    int		ungetch( int );

    int		handle() const;

    QString	errorString() const; // ### Qt 4: move into QIODevice

protected:
    void	setErrorString( const QString& ); // ### Qt 4: move into QIODevice
    QString	fn;
    FILE       *fh;
    int		fd;
    Offset	length;
    bool	ext_f;
    QFilePrivate *d; // ### Qt 4: make private

private:
    void	init();
    void	setErrorStringErrno( int );
    QCString	ungetchBuffer;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QFile( const QFile & );
    QFile &operator=( const QFile & );
#endif
};


inline QString QFile::name() const
{ return fn; }

inline QIODevice::Offset QFile::at() const
{ return ioIndex; }


#endif // QFILE_H
