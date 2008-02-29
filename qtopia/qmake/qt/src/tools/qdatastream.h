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

#ifndef QDATASTREAM_H
#define QDATASTREAM_H

#ifndef QT_H
#include "qiodevice.h"
#include "qstring.h"
#endif // QT_H

#ifndef QT_NO_DATASTREAM
class Q_EXPORT QDataStream				// data stream class
{
public:
    QDataStream();
    QDataStream( QIODevice * );
    QDataStream( QByteArray, int mode );
    virtual ~QDataStream();

    QIODevice	*device() const;
    void	 setDevice( QIODevice * );
    void	 unsetDevice();

    bool	 atEnd() const;
    bool	 eof() const;

    enum ByteOrder { BigEndian, LittleEndian };
    int		 byteOrder()	const;
    void	 setByteOrder( int );

    bool	 isPrintableData() const;
    void	 setPrintableData( bool );

    int		 version() const;
    void	 setVersion( int );

    QDataStream &operator>>( Q_INT8 &i );
    QDataStream &operator>>( Q_UINT8 &i );
    QDataStream &operator>>( Q_INT16 &i );
    QDataStream &operator>>( Q_UINT16 &i );
    QDataStream &operator>>( Q_INT32 &i );
    QDataStream &operator>>( Q_UINT32 &i );
    QDataStream &operator>>( Q_INT64 &i );
    QDataStream &operator>>( Q_UINT64 &i );
    QDataStream &operator>>( Q_LONG &i );
    QDataStream &operator>>( Q_ULONG &i );

    QDataStream &operator>>( float &f );
    QDataStream &operator>>( double &f );
    QDataStream &operator>>( char *&str );

    QDataStream &operator<<( Q_INT8 i );
    QDataStream &operator<<( Q_UINT8 i );
    QDataStream &operator<<( Q_INT16 i );
    QDataStream &operator<<( Q_UINT16 i );
    QDataStream &operator<<( Q_INT32 i );
    QDataStream &operator<<( Q_UINT32 i );
    QDataStream &operator<<( Q_INT64 i );
    QDataStream &operator<<( Q_UINT64 i );
    QDataStream &operator<<( Q_LONG i );
    QDataStream &operator<<( Q_ULONG i );
    QDataStream &operator<<( float f );
    QDataStream &operator<<( double f );
    QDataStream &operator<<( const char *str );

    QDataStream &readBytes( char *&, uint &len );
    QDataStream &readRawBytes( char *, uint len );

    QDataStream &writeBytes( const char *, uint len );
    QDataStream &writeRawBytes( const char *, uint len );

private:
    QIODevice	*dev;
    bool	 owndev;
    int		 byteorder;
    bool	 printable;
    bool	 noswap;
    int		 ver;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QDataStream( const QDataStream & );
    QDataStream &operator=( const QDataStream & );
#endif
};


/*****************************************************************************
  QDataStream inline functions
 *****************************************************************************/

inline QIODevice *QDataStream::device() const
{ return dev; }

inline bool QDataStream::atEnd() const
{ return dev ? dev->atEnd() : TRUE; }

inline bool QDataStream::eof() const
{ return atEnd(); }

inline int QDataStream::byteOrder() const
{ return byteorder; }

inline bool QDataStream::isPrintableData() const
{ return printable; }

inline void QDataStream::setPrintableData( bool p )
{ printable = p; }

inline int QDataStream::version() const
{ return ver; }

inline void QDataStream::setVersion( int v )
{ ver = v; }

inline QDataStream &QDataStream::operator>>( Q_UINT8 &i )
{ return *this >> (Q_INT8&)i; }

inline QDataStream &QDataStream::operator>>( Q_UINT16 &i )
{ return *this >> (Q_INT16&)i; }

inline QDataStream &QDataStream::operator>>( Q_UINT32 &i )
{ return *this >> (Q_INT32&)i; }

inline QDataStream &QDataStream::operator>>( Q_UINT64 &i )
{ return *this >> (Q_INT64&)i; }

inline QDataStream &QDataStream::operator>>( Q_ULONG &i )
{ return *this >> (Q_LONG&)i; }

inline QDataStream &QDataStream::operator<<( Q_UINT8 i )
{ return *this << (Q_INT8)i; }

inline QDataStream &QDataStream::operator<<( Q_UINT16 i )
{ return *this << (Q_INT16)i; }

inline QDataStream &QDataStream::operator<<( Q_UINT32 i )
{ return *this << (Q_INT32)i; }

inline QDataStream &QDataStream::operator<<( Q_UINT64 i )
{ return *this << (Q_INT64)i; }

inline QDataStream &QDataStream::operator<<( Q_ULONG i )
{ return *this << (Q_LONG)i; }


#endif // QT_NO_DATASTREAM
#endif // QDATASTREAM_H
