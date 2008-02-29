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

#ifndef QTEXTSTREAM_H
#define QTEXTSTREAM_H

#ifndef QT_H
#include "qiodevice.h"
#include "qstring.h"
#include <stdio.h>
#endif // QT_H

#ifndef QT_NO_TEXTSTREAM
class QTextCodec;
class QTextDecoder;

class QTextStreamPrivate;

class Q_EXPORT QTextStream				// text stream class
{
public:
    enum Encoding { Locale, Latin1, Unicode, UnicodeNetworkOrder,
		    UnicodeReverse, RawUnicode, UnicodeUTF8 };

    void	setEncoding( Encoding );
#ifndef QT_NO_TEXTCODEC
    void	setCodec( QTextCodec* );
    QTextCodec *codec();
#endif

    QTextStream();
    QTextStream( QIODevice * );
    QTextStream( QString*, int mode );
    QTextStream( QString&, int mode );		// obsolete
    QTextStream( QByteArray, int mode );
    QTextStream( FILE *, int mode );
    virtual ~QTextStream();

    QIODevice	*device() const;
    void	 setDevice( QIODevice * );
    void	 unsetDevice();

    bool	 atEnd() const;
    bool	 eof() const;

    QTextStream &operator>>( QChar & );
    QTextStream &operator>>( char & );
    QTextStream &operator>>( signed short & );
    QTextStream &operator>>( unsigned short & );
    QTextStream &operator>>( signed int & );
    QTextStream &operator>>( unsigned int & );
    QTextStream &operator>>( signed long & );
    QTextStream &operator>>( unsigned long & );
    QTextStream &operator>>( float & );
    QTextStream &operator>>( double & );
    QTextStream &operator>>( char * );
    QTextStream &operator>>( QString & );
    QTextStream &operator>>( QCString & );

    QTextStream &operator<<( QChar );
    QTextStream &operator<<( char );
    QTextStream &operator<<( signed short );
    QTextStream &operator<<( unsigned short );
    QTextStream &operator<<( signed int );
    QTextStream &operator<<( unsigned int );
    QTextStream &operator<<( signed long );
    QTextStream &operator<<( unsigned long );
    QTextStream &operator<<( float );
    QTextStream &operator<<( double );
    QTextStream &operator<<( const char* );
    QTextStream &operator<<( const QString & );
    QTextStream &operator<<( const QCString & );
    QTextStream &operator<<( void * );		// any pointer

    QTextStream &readRawBytes( char *, uint len );
    QTextStream &writeRawBytes( const char* , uint len );

    QString	readLine();
    QString	read();
    void	skipWhiteSpace();

    enum {
	skipws	  = 0x0001,			// skip whitespace on input
	left	  = 0x0002,			// left-adjust output
	right	  = 0x0004,			// right-adjust output
	internal  = 0x0008,			// pad after sign
	bin	  = 0x0010,			// binary format integer
	oct	  = 0x0020,			// octal format integer
	dec	  = 0x0040,			// decimal format integer
	hex	  = 0x0080,			// hex format integer
	showbase  = 0x0100,			// show base indicator
	showpoint = 0x0200,			// force decimal point (float)
	uppercase = 0x0400,			// upper-case hex output
	showpos	  = 0x0800,			// add '+' to positive integers
	scientific= 0x1000,			// scientific float output
	fixed	  = 0x2000			// fixed float output
    };

    static const int basefield;			// bin | oct | dec | hex
    static const int adjustfield;		// left | right | internal
    static const int floatfield;		// scientific | fixed

    int	  flags() const;
    int	  flags( int f );
    int	  setf( int bits );
    int	  setf( int bits, int mask );
    int	  unsetf( int bits );

    void  reset();

    int	  width()	const;
    int	  width( int );
    int	  fill()	const;
    int	  fill( int );
    int	  precision()	const;
    int	  precision( int );

private:
    long	input_int();
    void	init();
    QTextStream &output_int( int, ulong, bool );
    QIODevice	*dev;

    int		fflags;
    int		fwidth;
    int		fillchar;
    int		fprec;
    bool	doUnicodeHeader;
    bool	owndev;
    QTextCodec 	*mapper;
    QTextStreamPrivate * d;
    QChar	unused1; // ### remove in Qt 4.0
    bool	latin1;
    bool 	internalOrder;
    bool	networkOrder;
    void	*unused2; // ### remove in Qt 4.0

    QChar	eat_ws();
    uint 	ts_getline( QChar* );
    void	ts_ungetc( QChar );
    QChar	ts_getc();
    uint	ts_getbuf( QChar*, uint );
    void	ts_putc(int);
    void	ts_putc(QChar);
    bool	ts_isspace(QChar);
    bool	ts_isdigit(QChar);
    ulong	input_bin();
    ulong	input_oct();
    ulong	input_dec();
    ulong	input_hex();
    double	input_double();
    QTextStream &writeBlock( const char* p, uint len );
    QTextStream &writeBlock( const QChar* p, uint len );

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QTextStream( const QTextStream & );
    QTextStream &operator=( const QTextStream & );
#endif
};

typedef QTextStream QTS;

class Q_EXPORT QTextIStream : public QTextStream {
public:
    QTextIStream( const QString* s ) :
	QTextStream((QString*)s,IO_ReadOnly) { }
    QTextIStream( QByteArray ba ) :
	QTextStream(ba,IO_ReadOnly) { }
    QTextIStream( FILE *f ) :
	QTextStream(f,IO_ReadOnly) { }

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QTextIStream( const QTextIStream & );
    QTextIStream &operator=( const QTextIStream & );
#endif
};

class Q_EXPORT QTextOStream : public QTextStream {
public:
    QTextOStream( QString* s ) :
	QTextStream(s,IO_WriteOnly) { }
    QTextOStream( QByteArray ba ) :
	QTextStream(ba,IO_WriteOnly) { }
    QTextOStream( FILE *f ) :
	QTextStream(f,IO_WriteOnly) { }

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QTextOStream( const QTextOStream & );
    QTextOStream &operator=( const QTextOStream & );
#endif
};

/*****************************************************************************
  QTextStream inline functions
 *****************************************************************************/

inline QIODevice *QTextStream::device() const
{ return dev; }

inline bool QTextStream::atEnd() const
{ return dev ? dev->atEnd() : FALSE; }

inline bool QTextStream::eof() const
{ return atEnd(); }

inline int QTextStream::flags() const
{ return fflags; }

inline int QTextStream::flags( int f )
{ int oldf = fflags;  fflags = f;  return oldf; }

inline int QTextStream::setf( int bits )
{ int oldf = fflags;  fflags |= bits;  return oldf; }

inline int QTextStream::setf( int bits, int mask )
{ int oldf = fflags;  fflags = (fflags & ~mask) | (bits & mask); return oldf; }

inline int QTextStream::unsetf( int bits )
{ int oldf = fflags;  fflags &= ~bits;	return oldf; }

inline int QTextStream::width() const
{ return fwidth; }

inline int QTextStream::width( int w )
{ int oldw = fwidth;  fwidth = w;  return oldw;	 }

inline int QTextStream::fill() const
{ return fillchar; }

inline int QTextStream::fill( int f )
{ int oldc = fillchar;	fillchar = f;  return oldc;  }

inline int QTextStream::precision() const
{ return fprec; }

inline int QTextStream::precision( int p )
{ int oldp = fprec;  fprec = p;	 return oldp;  }

/*!
  Returns one character from the stream, or EOF.
*/
inline QChar QTextStream::ts_getc()
{ QChar r; return ( ts_getbuf( &r,1 ) == 1 ? r : QChar((ushort)0xffff) ); }

/*****************************************************************************
  QTextStream manipulators
 *****************************************************************************/

typedef QTextStream & (*QTSFUNC)(QTextStream &);// manipulator function
typedef int (QTextStream::*QTSMFI)(int);	// manipulator w/int argument

class Q_EXPORT QTSManip {			// text stream manipulator
public:
    QTSManip( QTSMFI m, int a ) { mf=m; arg=a; }
    void exec( QTextStream &s ) { (s.*mf)(arg); }
private:
    QTSMFI mf;					// QTextStream member function
    int	   arg;					// member function argument
};

Q_EXPORT inline QTextStream &operator>>( QTextStream &s, QTSFUNC f )
{ return (*f)( s ); }

Q_EXPORT inline QTextStream &operator<<( QTextStream &s, QTSFUNC f )
{ return (*f)( s ); }

Q_EXPORT inline QTextStream &operator<<( QTextStream &s, QTSManip m )
{ m.exec(s); return s; }

Q_EXPORT QTextStream &bin( QTextStream &s );	// set bin notation
Q_EXPORT QTextStream &oct( QTextStream &s );	// set oct notation
Q_EXPORT QTextStream &dec( QTextStream &s );	// set dec notation
Q_EXPORT QTextStream &hex( QTextStream &s );	// set hex notation
Q_EXPORT QTextStream &endl( QTextStream &s );	// insert EOL ('\n')
Q_EXPORT QTextStream &flush( QTextStream &s );	// flush output
Q_EXPORT QTextStream &ws( QTextStream &s );	// eat whitespace on input
Q_EXPORT QTextStream &reset( QTextStream &s );	// set default flags

Q_EXPORT inline QTSManip qSetW( int w )
{
    QTSMFI func = &QTextStream::width;
    return QTSManip(func,w);
}

Q_EXPORT inline QTSManip qSetFill( int f )
{
    QTSMFI func = &QTextStream::fill;
    return QTSManip(func,f);
}

Q_EXPORT inline QTSManip qSetPrecision( int p )
{
    QTSMFI func = &QTextStream::precision;
    return QTSManip(func,p);
}

#endif // QT_NO_TEXTSTREAM
#endif // QTEXTSTREAM_H
