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

#ifndef QINTCACHE_H
#define QINTCACHE_H

#ifndef QT_H
#include "qgcache.h"
#endif // QT_H


template<class type> 
class QIntCache : public QGCache
{
public:
    QIntCache( const QIntCache<type> &c ) : QGCache(c) {}
    QIntCache( int maxCost=100, int size=17 )
	: QGCache( maxCost, size, IntKey, FALSE, FALSE ) {}
   ~QIntCache()		{ clear(); }
    QIntCache<type> &operator=( const QIntCache<type> &c )
			{ return (QIntCache<type>&)QGCache::operator=(c); }
    int	  maxCost()   const	{ return QGCache::maxCost(); }
    int	  totalCost() const	{ return QGCache::totalCost(); }
    void  setMaxCost( int m)	{ QGCache::setMaxCost(m); }
    uint  count()     const	{ return QGCache::count(); }
    uint  size()      const	{ return QGCache::size(); }
    bool  isEmpty()   const	{ return QGCache::count() == 0; }
    bool  insert( long k, const type *d, int c=1, int p=0 )
		{ return QGCache::insert_other((const char*)k,(Item)d,c,p); }
    bool  remove( long k )
		{ return QGCache::remove_other((const char*)k); }
    type *take( long k )
		{ return (type *)QGCache::take_other((const char*)k);}
    void  clear()		{ QGCache::clear(); }
    type *find( long k, bool ref=TRUE ) const
		{ return (type *)QGCache::find_other( (const char*)k,ref);}
    type *operator[]( long k ) const
		{ return (type *)QGCache::find_other( (const char*)k); }
    void  statistics() const { QGCache::statistics(); }
private:
	void  deleteItem( Item d );
};

#if !defined(Q_BROKEN_TEMPLATE_SPECIALIZATION)
template<> inline void QIntCache<void>::deleteItem( QPtrCollection::Item )
{
}
#endif

template<class type> inline void QIntCache<type>::deleteItem( QPtrCollection::Item d )
{
    if ( del_item ) delete (type *)d;
}

template<class type> 
class QIntCacheIterator : public QGCacheIterator
{
public:
    QIntCacheIterator( const QIntCache<type> &c )
	: QGCacheIterator( (QGCache &)c ) {}
    QIntCacheIterator( const QIntCacheIterator<type> &ci )
			      : QGCacheIterator((QGCacheIterator &)ci) {}
    QIntCacheIterator<type> &operator=( const QIntCacheIterator<type>&ci )
	{ return ( QIntCacheIterator<type>&)QGCacheIterator::operator=( ci );}
    uint  count()   const     { return QGCacheIterator::count(); }
    bool  isEmpty() const     { return QGCacheIterator::count() == 0; }
    bool  atFirst() const     { return QGCacheIterator::atFirst(); }
    bool  atLast()  const     { return QGCacheIterator::atLast(); }
    type *toFirst()	      { return (type *)QGCacheIterator::toFirst(); }
    type *toLast()	      { return (type *)QGCacheIterator::toLast(); }
    operator type *()  const  { return (type *)QGCacheIterator::get(); }
    type *current()    const  { return (type *)QGCacheIterator::get(); }
    long  currentKey() const  { return (long)QGCacheIterator::getKeyInt();}
    type *operator()()	      { return (type *)QGCacheIterator::operator()();}
    type *operator++()	      { return (type *)QGCacheIterator::operator++(); }
    type *operator+=(uint j)  { return (type *)QGCacheIterator::operator+=(j);}
    type *operator--()	      { return (type *)QGCacheIterator::operator--(); }
    type *operator-=(uint j)  { return (type *)QGCacheIterator::operator-=(j);}
};


#endif // QINTCACHE_H
