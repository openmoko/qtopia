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

#ifndef QCSTRING_H
#define QCSTRING_H

#ifndef QT_H
#include "qmemarray.h"
#endif // QT_H

#include <string.h>


/*****************************************************************************
  Safe and portable C string functions; extensions to standard string.h
 *****************************************************************************/

Q_EXPORT void *qmemmove( void *dst, const void *src, uint len );

Q_EXPORT char *qstrdup( const char * );

Q_EXPORT inline uint qstrlen( const char *str )
{ return str ? (uint)strlen(str) : 0u; }

Q_EXPORT inline char *qstrcpy( char *dst, const char *src )
{ return src ? strcpy(dst, src) : 0; }

Q_EXPORT char *qstrncpy( char *dst, const char *src, uint len );

Q_EXPORT inline int qstrcmp( const char *str1, const char *str2 )
{
    return ( str1 && str2 ) ? strcmp( str1, str2 )
			    : ( str1 ? 1 : ( str2 ? -1 : 0 ) );
}

Q_EXPORT inline int qstrncmp( const char *str1, const char *str2, uint len )
{
    return ( str1 && str2 ) ? strncmp( str1, str2, len )
			    : ( str1 ? 1 : ( str2 ? -1 : 0 ) );
}

Q_EXPORT int qstricmp( const char *, const char * );

Q_EXPORT int qstrnicmp( const char *, const char *, uint len );

#ifndef QT_CLEAN_NAMESPACE
Q_EXPORT inline uint cstrlen( const char *str )
{ return (uint)strlen(str); }

Q_EXPORT inline char *cstrcpy( char *dst, const char *src )
{ return strcpy(dst,src); }

Q_EXPORT inline int cstrcmp( const char *str1, const char *str2 )
{ return strcmp(str1,str2); }

Q_EXPORT inline int cstrncmp( const char *str1, const char *str2, uint len )
{ return strncmp(str1,str2,len); }
#endif


// qChecksum: Internet checksum

Q_EXPORT Q_UINT16 qChecksum( const char *s, uint len );

/*****************************************************************************
  QByteArray class
 *****************************************************************************/

#if defined(Q_QDOC)
/*
  We want qdoc to document QByteArray as a real class that inherits
  QMemArray<char> and that is inherited by QBitArray.
*/
class QByteArray : public QMemArray<char>
{
public:
    QByteArray();
    QByteArray( int size );
};
#else
typedef QMemArray<char> QByteArray;
#endif

#ifndef QT_NO_COMPRESS
Q_EXPORT QByteArray qCompress( const uchar* data, int nbytes );
Q_EXPORT QByteArray qUncompress( const uchar* data, int nbytes );
Q_EXPORT inline QByteArray qCompress( const QByteArray& data)
{ return qCompress( (const uchar*)data.data(), data.size() ); }
Q_EXPORT inline QByteArray qUncompress( const QByteArray& data )
{ return qUncompress( (const uchar*)data.data(), data.size() ); }
#endif

/*****************************************************************************
  QByteArray stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
Q_EXPORT QDataStream &operator<<( QDataStream &, const QByteArray & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QByteArray & );
#endif

/*****************************************************************************
  QCString class
 *****************************************************************************/

class QRegExp;

class Q_EXPORT QCString : public QByteArray	// C string class
{
public:
    QCString() {}				// make null string
    QCString( int size );			// allocate size incl. \0
    QCString( const QCString &s ) : QByteArray( s ) {}
    QCString( const char *str );		// deep copy
    QCString( const char *str, uint maxlen );	// deep copy, max length
    ~QCString();

    QCString    &operator=( const QCString &s );// shallow copy
    QCString    &operator=( const char *str );	// deep copy

    bool	isNull()	const;
    bool	isEmpty()	const;
    uint	length()	const;
    bool	resize( uint newlen );
    bool	truncate( uint pos );
    bool	fill( char c, int len = -1 );

    QCString	copy()	const;

    QCString    &sprintf( const char *format, ... );

    int		find( char c, int index=0, bool cs=TRUE ) const;
    int		find( const char *str, int index=0, bool cs=TRUE ) const;
#ifndef QT_NO_REGEXP
    int		find( const QRegExp &, int index=0 ) const;
#endif
    int		findRev( char c, int index=-1, bool cs=TRUE) const;
    int		findRev( const char *str, int index=-1, bool cs=TRUE) const;
#ifndef QT_NO_REGEXP_CAPTURE
    int		findRev( const QRegExp &, int index=-1 ) const;
#endif
    int		contains( char c, bool cs=TRUE ) const;
    int		contains( const char *str, bool cs=TRUE ) const;
#ifndef QT_NO_REGEXP
    int		contains( const QRegExp & ) const;
#endif
    QCString	left( uint len )  const;
    QCString	right( uint len ) const;
    QCString	mid( uint index, uint len=0xffffffff) const;

    QCString	leftJustify( uint width, char fill=' ', bool trunc=FALSE)const;
    QCString	rightJustify( uint width, char fill=' ',bool trunc=FALSE)const;

    QCString	lower() const;
    QCString	upper() const;

    QCString	stripWhiteSpace()	const;
    QCString	simplifyWhiteSpace()	const;

    QCString    &insert( uint index, const char * );
    QCString    &insert( uint index, char );
    QCString    &append( const char * );
    QCString    &prepend( const char * );
    QCString    &remove( uint index, uint len );
    QCString    &replace( uint index, uint len, const char * );
#ifndef QT_NO_REGEXP
    QCString    &replace( const QRegExp &, const char * );
#endif
    QCString    &replace( char c, const char *after );
    QCString    &replace( const char *, const char * );
    QCString    &replace( char, char );

    short	toShort( bool *ok=0 )	const;
    ushort	toUShort( bool *ok=0 )	const;
    int		toInt( bool *ok=0 )	const;
    uint	toUInt( bool *ok=0 )	const;
    long	toLong( bool *ok=0 )	const;
    ulong	toULong( bool *ok=0 )	const;
    float	toFloat( bool *ok=0 )	const;
    double	toDouble( bool *ok=0 )	const;

    QCString    &setStr( const char *s );
    QCString    &setNum( short );
    QCString    &setNum( ushort );
    QCString    &setNum( int );
    QCString    &setNum( uint );
    QCString    &setNum( long );
    QCString    &setNum( ulong );
    QCString    &setNum( float, char f='g', int prec=6 );
    QCString    &setNum( double, char f='g', int prec=6 );

    bool	setExpand( uint index, char c );

		operator const char *() const;
    QCString    &operator+=( const char *str );
    QCString    &operator+=( char c );
private:
    int	find( const char *str, int index, bool cs, uint l ) const;
};


/*****************************************************************************
  QCString stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
Q_EXPORT QDataStream &operator<<( QDataStream &, const QCString & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QCString & );
#endif

/*****************************************************************************
  QCString inline functions
 *****************************************************************************/

inline QCString &QCString::operator=( const QCString &s )
{ return (QCString&)assign( s ); }

inline QCString &QCString::operator=( const char *str )
{ return (QCString&)duplicate( str, qstrlen(str)+1 ); }

inline bool QCString::isNull() const
{ return data() == 0; }

inline bool QCString::isEmpty() const
{ return data() == 0 || *data() == '\0'; }

inline uint QCString::length() const
{ return qstrlen( data() ); }

inline bool QCString::truncate( uint pos )
{ return resize(pos+1); }

inline QCString QCString::copy() const
{ return QCString( data() ); }

inline QCString &QCString::prepend( const char *s )
{ return insert(0,s); }

inline QCString &QCString::append( const char *s )
{ return operator+=(s); }

inline QCString &QCString::setNum( short n )
{ return setNum((long)n); }

inline QCString &QCString::setNum( ushort n )
{ return setNum((ulong)n); }

inline QCString &QCString::setNum( int n )
{ return setNum((long)n); }

inline QCString &QCString::setNum( uint n )
{ return setNum((ulong)n); }

inline QCString &QCString::setNum( float n, char f, int prec )
{ return setNum((double)n,f,prec); }

inline QCString::operator const char *() const
{ return (const char *)data(); }


/*****************************************************************************
  QCString non-member operators
 *****************************************************************************/

Q_EXPORT inline bool operator==( const QCString &s1, const QCString &s2 )
{ return qstrcmp( s1.data(), s2.data() ) == 0; }

Q_EXPORT inline bool operator==( const QCString &s1, const char *s2 )
{ return qstrcmp( s1.data(), s2 ) == 0; }

Q_EXPORT inline bool operator==( const char *s1, const QCString &s2 )
{ return qstrcmp( s1, s2.data() ) == 0; }

Q_EXPORT inline bool operator!=( const QCString &s1, const QCString &s2 )
{ return qstrcmp( s1.data(), s2.data() ) != 0; }

Q_EXPORT inline bool operator!=( const QCString &s1, const char *s2 )
{ return qstrcmp( s1.data(), s2 ) != 0; }

Q_EXPORT inline bool operator!=( const char *s1, const QCString &s2 )
{ return qstrcmp( s1, s2.data() ) != 0; }

Q_EXPORT inline bool operator<( const QCString &s1, const QCString& s2 )
{ return qstrcmp( s1.data(), s2.data() ) < 0; }

Q_EXPORT inline bool operator<( const QCString &s1, const char *s2 )
{ return qstrcmp( s1.data(), s2 ) < 0; }

Q_EXPORT inline bool operator<( const char *s1, const QCString &s2 )
{ return qstrcmp( s1, s2.data() ) < 0; }

Q_EXPORT inline bool operator<=( const QCString &s1, const QCString &s2 )
{ return qstrcmp( s1.data(), s2.data() ) <= 0; }

Q_EXPORT inline bool operator<=( const QCString &s1, const char *s2 )
{ return qstrcmp( s1.data(), s2 ) <= 0; }

Q_EXPORT inline bool operator<=( const char *s1, const QCString &s2 )
{ return qstrcmp( s1, s2.data() ) <= 0; }

Q_EXPORT inline bool operator>( const QCString &s1, const QCString &s2 )
{ return qstrcmp( s1.data(), s2.data() ) > 0; }

Q_EXPORT inline bool operator>( const QCString &s1, const char *s2 )
{ return qstrcmp( s1.data(), s2 ) > 0; }

Q_EXPORT inline bool operator>( const char *s1, const QCString &s2 )
{ return qstrcmp( s1, s2.data() ) > 0; }

Q_EXPORT inline bool operator>=( const QCString &s1, const QCString& s2 )
{ return qstrcmp( s1.data(), s2.data() ) >= 0; }

Q_EXPORT inline bool operator>=( const QCString &s1, const char *s2 )
{ return qstrcmp( s1.data(), s2 ) >= 0; }

Q_EXPORT inline bool operator>=( const char *s1, const QCString &s2 )
{ return qstrcmp( s1, s2.data() ) >= 0; }

Q_EXPORT inline const QCString operator+( const QCString &s1,
					  const QCString &s2 )
{
    QCString tmp( s1.data() );
    tmp += s2;
    return tmp;
}

Q_EXPORT inline const QCString operator+( const QCString &s1, const char *s2 )
{
    QCString tmp( s1.data() );
    tmp += s2;
    return tmp;
}

Q_EXPORT inline const QCString operator+( const char *s1, const QCString &s2 )
{
    QCString tmp( s1 );
    tmp += s2;
    return tmp;
}

Q_EXPORT inline const QCString operator+( const QCString &s1, char c2 )
{
    QCString tmp( s1.data() );
    tmp += c2;
    return tmp;
}

Q_EXPORT inline const QCString operator+( char c1, const QCString &s2 )
{
    QCString tmp;
    tmp += c1;
    tmp += s2;
    return tmp;
}
#include "qwinexport.h"
#endif // QCSTRING_H
