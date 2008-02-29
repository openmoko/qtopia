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

#ifndef QBUFFER_H
#define QBUFFER_H

#ifndef QT_H
#include "qiodevice.h"
#include "qstring.h"
#endif // QT_H


class Q_EXPORT QBuffer : public QIODevice
{
public:
    QBuffer();
    QBuffer( QByteArray );
   ~QBuffer();

    QByteArray buffer() const;
    bool  setBuffer( QByteArray );

    bool  open( int );
    void  close();
    void  flush();

    Offset size() const;
    Offset at() const;
    bool  at( Offset );

    Q_LONG	  readBlock( char *p, Q_ULONG );
    Q_LONG	  writeBlock( const char *p, Q_ULONG );
    Q_LONG	  writeBlock( const QByteArray& data )
	      { return QIODevice::writeBlock(data); }
    Q_LONG	  readLine( char *p, Q_ULONG );

    int	  getch();
    int	  putch( int );
    int	  ungetch( int );

protected:
    QByteArray a;

private:
    uint  a_len;
    uint  a_inc;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QBuffer( const QBuffer & );
    QBuffer &operator=( const QBuffer & );
#endif
};


inline QByteArray QBuffer::buffer() const
{ return a; }

inline QIODevice::Offset QBuffer::size() const
{ return (Offset)a.size(); }

inline QIODevice::Offset QBuffer::at() const
{ return ioIndex; }


#endif // QBUFFER_H
