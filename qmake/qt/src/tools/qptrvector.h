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

#ifndef QPTRVECTOR_H
#define QPTRVECTOR_H

#ifndef QT_H
#include "qgvector.h"
#endif // QT_H

template<class type>
class QPtrVector
#ifdef Q_QDOC
	: public QPtrCollection
#else
	: public QGVector
#endif
{
public:
    QPtrVector()				{ }
    QPtrVector( uint size ) : QGVector(size) { }
    QPtrVector( const QPtrVector<type> &v ) : QGVector( v ) { }
    ~QPtrVector()				{ clear(); }
    QPtrVector<type> &operator=(const QPtrVector<type> &v)
			{ return (QPtrVector<type>&)QGVector::operator=(v); }
    bool operator==( const QPtrVector<type> &v ) const { return QGVector::operator==(v); }
    type **data()   const		{ return (type **)QGVector::data(); }
    uint  size()    const		{ return QGVector::size(); }
    uint  count()   const		{ return QGVector::count(); }
    bool  isEmpty() const		{ return QGVector::count() == 0; }
    bool  isNull()  const		{ return QGVector::size() == 0; }
    bool  resize( uint size )		{ return QGVector::resize(size); }
    bool  insert( uint i, const type *d){ return QGVector::insert(i,(Item)d); }
    bool  remove( uint i )		{ return QGVector::remove(i); }
    type *take( uint i )		{ return (type *)QGVector::take(i); }
    void  clear()			{ QGVector::clear(); }
    bool  fill( const type *d, int size=-1 )
					{ return QGVector::fill((Item)d,size);}
    void  sort()			{ QGVector::sort(); }
    int	  bsearch( const type *d ) const{ return QGVector::bsearch((Item)d); }
    int	  findRef( const type *d, uint i=0 ) const
					{ return QGVector::findRef((Item)d,i);}
    int	  find( const type *d, uint i= 0 ) const
					{ return QGVector::find((Item)d,i); }
    uint  containsRef( const type *d ) const
				{ return QGVector::containsRef((Item)d); }
    uint  contains( const type *d ) const
					{ return QGVector::contains((Item)d); }
    type *operator[]( int i ) const	{ return (type *)QGVector::at(i); }
    type *at( uint i ) const		{ return (type *)QGVector::at(i); }
    void  toList( QGList *list ) const	{ QGVector::toList(list); }

#ifdef Q_QDOC
protected:
    virtual int compareItems( QPtrCollection::Item d1, QPtrCollection::Item d2 );
    virtual QDataStream& read( QDataStream &s, QPtrCollection::Item &d );
    virtual QDataStream& write( QDataStream &s, QPtrCollection::Item d ) const;
#endif

private:
    void  deleteItem( Item d );
};

#if !defined(Q_BROKEN_TEMPLATE_SPECIALIZATION)
template<> inline void QPtrVector<void>::deleteItem( QPtrCollection::Item )
{
}
#endif

template<class type> inline void QPtrVector<type>::deleteItem( QPtrCollection::Item d )
{
    if ( del_item ) delete (type *)d;
}

#ifndef QT_NO_COMPAT
#define QVector QPtrVector
#endif

#define Q_DEFINED_QPTRVECTOR
#include "qwinexport.h"
#endif // QVECTOR_H
