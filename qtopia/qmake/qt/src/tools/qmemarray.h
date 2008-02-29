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

#ifndef QMEMARRAY_H
#define QMEMARRAY_H

#ifndef QT_H
#include "qgarray.h"
#endif // QT_H


template<class type>
class QMemArray : public QGArray
{
public:
    typedef type* Iterator;
    typedef const type* ConstIterator;
    typedef type ValueType;

protected:
    QMemArray( int, int ) : QGArray( 0, 0 ) {}

public:
    QMemArray() {}
    QMemArray( int size ) : QGArray(size*sizeof(type)) {} // ### 4.0 Q_EXPLICIT
    QMemArray( const QMemArray<type> &a ) : QGArray(a) {}
   ~QMemArray() {}
    QMemArray<type> &operator=(const QMemArray<type> &a)
				{ return (QMemArray<type>&)QGArray::assign(a); }
    type *data()    const	{ return (type *)QGArray::data(); }
    uint  nrefs()   const	{ return QGArray::nrefs(); }
    uint  size()    const	{ return QGArray::size()/sizeof(type); }
    uint  count()   const	{ return size(); }
    bool  isEmpty() const	{ return QGArray::size() == 0; }
    bool  isNull()  const	{ return QGArray::data() == 0; }
    bool  resize( uint size )	{ return QGArray::resize(size*sizeof(type)); }
    bool  resize( uint size, Optimization optim ) { return QGArray::resize(size*sizeof(type), optim); }
    bool  truncate( uint pos )	{ return QGArray::resize(pos*sizeof(type)); }
    bool  fill( const type &d, int size = -1 )
	{ return QGArray::fill((char*)&d,size,sizeof(type) ); }
    void  detach()		{ QGArray::detach(); }
    QMemArray<type>   copy() const
	{ QMemArray<type> tmp; return tmp.duplicate(*this); }
    QMemArray<type>& assign( const QMemArray<type>& a )
	{ return (QMemArray<type>&)QGArray::assign(a); }
    QMemArray<type>& assign( const type *a, uint n )
	{ return (QMemArray<type>&)QGArray::assign((char*)a,n*sizeof(type)); }
    QMemArray<type>& duplicate( const QMemArray<type>& a )
	{ return (QMemArray<type>&)QGArray::duplicate(a); }
    QMemArray<type>& duplicate( const type *a, uint n )
	{ return (QMemArray<type>&)QGArray::duplicate((char*)a,n*sizeof(type)); }
    QMemArray<type>& setRawData( const type *a, uint n )
	{ return (QMemArray<type>&)QGArray::setRawData((char*)a,
						     n*sizeof(type)); }
    void resetRawData( const type *a, uint n )
	{ QGArray::resetRawData((char*)a,n*sizeof(type)); }
    int	 find( const type &d, uint i=0 ) const
	{ return QGArray::find((char*)&d,i,sizeof(type)); }
    int	 contains( const type &d ) const
	{ return QGArray::contains((char*)&d,sizeof(type)); }
    void sort() { QGArray::sort(sizeof(type)); }
    int  bsearch( const type &d ) const
	{ return QGArray::bsearch((const char*)&d,sizeof(type)); }
    // ### Qt 4.0: maybe provide uint overload as work-around for MSVC bug
    type& operator[]( int i ) const
	{ return (type &)(*(type *)QGArray::at(i*sizeof(type))); }
    type& at( uint i ) const
	{ return (type &)(*(type *)QGArray::at(i*sizeof(type))); }
	 operator const type*() const { return (const type *)QGArray::data(); }
    bool operator==( const QMemArray<type> &a ) const { return isEqual(a); }
    bool operator!=( const QMemArray<type> &a ) const { return !isEqual(a); }
    Iterator begin() { return data(); }
    Iterator end() { return data() + size(); }
    ConstIterator begin() const { return data(); }
    ConstIterator end() const { return data() + size(); }
};

#ifndef QT_NO_COMPAT
#define QArray QMemArray
#endif

#define Q_DEFINED_QMEMARRAY
#include "qwinexport.h"
#endif // QARRAY_H
