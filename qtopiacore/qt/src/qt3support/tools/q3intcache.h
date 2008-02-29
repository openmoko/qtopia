/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef Q3INTCACHE_H
#define Q3INTCACHE_H

#include <Qt3Support/q3gcache.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

template<class type>
class Q3IntCache
#ifdef qdoc
	: public Q3PtrCollection
#else
	: public Q3GCache
#endif
{
public:
    Q3IntCache( const Q3IntCache<type> &c ) : Q3GCache(c) {}
    Q3IntCache( int maxCost=100, int size=17 )
	: Q3GCache( maxCost, size, IntKey, false, false ) {}
   ~Q3IntCache()		{ clear(); }
    Q3IntCache<type> &operator=( const Q3IntCache<type> &c )
			{ return (Q3IntCache<type>&)Q3GCache::operator=(c); }
    int	  maxCost()   const	{ return Q3GCache::maxCost(); }
    int	  totalCost() const	{ return Q3GCache::totalCost(); }
    void  setMaxCost( int m)	{ Q3GCache::setMaxCost(m); }
    uint  count()     const	{ return Q3GCache::count(); }
    uint  size()      const	{ return Q3GCache::size(); }
    bool  isEmpty()   const	{ return Q3GCache::count() == 0; }
    bool  insert( long k, const type *d, int c=1, int p=0 )
		{ return Q3GCache::insert_other((const char*)k,(Item)d,c,p); }
    bool  remove( long k )
		{ return Q3GCache::remove_other((const char*)k); }
    type *take( long k )
		{ return (type *)Q3GCache::take_other((const char*)k);}
    void  clear()		{ Q3GCache::clear(); }
    type *find( long k, bool ref=true ) const
		{ return (type *)Q3GCache::find_other( (const char*)k,ref);}
    type *operator[]( long k ) const
		{ return (type *)Q3GCache::find_other( (const char*)k); }
    void  statistics() const { Q3GCache::statistics(); }
private:
	void  deleteItem( Item d );
};

#if !defined(Q_BROKEN_TEMPLATE_SPECIALIZATION)
template<> inline void Q3IntCache<void>::deleteItem( Q3PtrCollection::Item )
{
}
#endif

template<class type> inline void Q3IntCache<type>::deleteItem( Q3PtrCollection::Item d )
{
    if ( del_item ) delete (type *)d;
}

template<class type>
class Q3IntCacheIterator : public Q3GCacheIterator
{
public:
    Q3IntCacheIterator( const Q3IntCache<type> &c )
	: Q3GCacheIterator( (Q3GCache &)c ) {}
    Q3IntCacheIterator( const Q3IntCacheIterator<type> &ci )
			      : Q3GCacheIterator((Q3GCacheIterator &)ci) {}
    Q3IntCacheIterator<type> &operator=( const Q3IntCacheIterator<type>&ci )
	{ return ( Q3IntCacheIterator<type>&)Q3GCacheIterator::operator=( ci );}
    uint  count()   const     { return Q3GCacheIterator::count(); }
    bool  isEmpty() const     { return Q3GCacheIterator::count() == 0; }
    bool  atFirst() const     { return Q3GCacheIterator::atFirst(); }
    bool  atLast()  const     { return Q3GCacheIterator::atLast(); }
    type *toFirst()	      { return (type *)Q3GCacheIterator::toFirst(); }
    type *toLast()	      { return (type *)Q3GCacheIterator::toLast(); }
    operator type *()  const  { return (type *)Q3GCacheIterator::get(); }
    type *current()    const  { return (type *)Q3GCacheIterator::get(); }
    long  currentKey() const  { return (long)Q3GCacheIterator::getKeyInt();}
    type *operator()()	      { return (type *)Q3GCacheIterator::operator()();}
    type *operator++()	      { return (type *)Q3GCacheIterator::operator++(); }
    type *operator+=(uint j)  { return (type *)Q3GCacheIterator::operator+=(j);}
    type *operator--()	      { return (type *)Q3GCacheIterator::operator--(); }
    type *operator-=(uint j)  { return (type *)Q3GCacheIterator::operator-=(j);}
};

QT_END_HEADER

#endif // Q3INTCACHE_H
