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

#ifndef QBITARRAY_H
#define QBITARRAY_H

#ifndef QT_H
#include "qstring.h"
#endif // QT_H


/*****************************************************************************
  QBitVal class; a context class for QBitArray::operator[]
 *****************************************************************************/

class QBitArray;

class Q_EXPORT QBitVal
{
private:
    QBitArray *array;
    uint    index;
public:
    QBitVal( QBitArray *a, uint i ) : array(a), index(i) {}
    operator int();
    QBitVal &operator=( const QBitVal &v );
    QBitVal &operator=( bool v );
};


/*****************************************************************************
  QBitArray class
 *****************************************************************************/

class Q_EXPORT QBitArray : public QByteArray
{
public:
    QBitArray();
    QBitArray( uint size );
    QBitArray( const QBitArray &a ) : QByteArray( a ) {}

    QBitArray &operator=( const QBitArray & );

    uint    size() const;
    bool    resize( uint size );

    bool    fill( bool v, int size = -1 );

    void    detach();
    QBitArray copy() const;

    bool    testBit( uint index ) const;
    void    setBit( uint index );
    void    setBit( uint index, bool value );
    void    clearBit( uint index );
    bool    toggleBit( uint index );

    bool    at( uint index ) const;
    QBitVal operator[]( int index );
    bool operator[]( int index ) const;

    QBitArray &operator&=( const QBitArray & );
    QBitArray &operator|=( const QBitArray & );
    QBitArray &operator^=( const QBitArray & );
    QBitArray  operator~() const;

protected:
    struct bitarr_data : public QGArray::array_data {
	uint   nbits;
    };
    array_data *newData()		    { return new bitarr_data; }
    void	deleteData( array_data *d ) { delete (bitarr_data*)d; }
private:
    void    pad0();
};


inline QBitArray &QBitArray::operator=( const QBitArray &a )
{ return (QBitArray&)assign( a ); }

inline uint QBitArray::size() const
{ return ((bitarr_data*)sharedBlock())->nbits; }

inline void QBitArray::setBit( uint index, bool value )
{ if ( value ) setBit(index); else clearBit(index); }

inline bool QBitArray::at( uint index ) const
{ return testBit(index); }

inline QBitVal QBitArray::operator[]( int index )
{ return QBitVal( (QBitArray*)this, index ); }

inline bool QBitArray::operator[]( int index ) const
{ return testBit( index ); }


/*****************************************************************************
  Misc. QBitArray operator functions
 *****************************************************************************/

Q_EXPORT QBitArray operator&( const QBitArray &, const QBitArray & );
Q_EXPORT QBitArray operator|( const QBitArray &, const QBitArray & );
Q_EXPORT QBitArray operator^( const QBitArray &, const QBitArray & );


inline QBitVal::operator int()
{
    return array->testBit( index );
}

inline QBitVal &QBitVal::operator=( const QBitVal &v )
{
    array->setBit( index, v.array->testBit(v.index) );
    return *this;
}

inline QBitVal &QBitVal::operator=( bool v )
{
    array->setBit( index, v );
    return *this;
}


/*****************************************************************************
  QBitArray stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
Q_EXPORT QDataStream &operator<<( QDataStream &, const QBitArray & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QBitArray & );
#endif

#endif // QBITARRAY_H
