/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef Q3ASCIICACHE_H
#define Q3ASCIICACHE_H

#include <Qt3Support/q3gcache.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

template<class type>
class Q3AsciiCache
#ifdef qdoc
	: public Q3PtrCollection
#else
	: public Q3GCache
#endif
{
public:
    Q3AsciiCache(const Q3AsciiCache<type> &c) : Q3GCache(c) {}
    Q3AsciiCache(int maxCost=100, int size=17, bool caseSensitive=true,
		 bool copyKeys=true)
	: Q3GCache(maxCost, size, AsciiKey, caseSensitive, copyKeys) {}
   ~Q3AsciiCache()			{ clear(); }
    Q3AsciiCache<type> &operator=(const Q3AsciiCache<type> &c)
			{ return (Q3AsciiCache<type>&)Q3GCache::operator=(c); }
    int	  maxCost()   const		{ return Q3GCache::maxCost(); }
    int	  totalCost() const		{ return Q3GCache::totalCost(); }
    void  setMaxCost(int m)		{ Q3GCache::setMaxCost(m); }
    uint  count()     const		{ return Q3GCache::count(); }
    uint  size()      const		{ return Q3GCache::size(); }
    bool  isEmpty()   const		{ return Q3GCache::count() == 0; }
    void  clear()			{ Q3GCache::clear(); }
    bool  insert(const char *k, const type *d, int c=1, int p=0)
			{ return Q3GCache::insert_other(k,(Item)d,c,p);}
    bool  remove(const char *k)
			{ return Q3GCache::remove_other(k); }
    type *take(const char *k)
			{ return (type *)Q3GCache::take_other(k); }
    type *find(const char *k, bool ref=true) const
			{ return (type *)Q3GCache::find_other(k,ref);}
    type *operator[](const char *k) const
			{ return (type *)Q3GCache::find_other(k);}
    void  statistics() const	      { Q3GCache::statistics(); }
private:
    void  deleteItem(Item d);
};

#if !defined(Q_BROKEN_TEMPLATE_SPECIALIZATION)
template<> inline void Q3AsciiCache<void>::deleteItem(Q3PtrCollection::Item)
{
}
#endif

template<class type> inline void Q3AsciiCache<type>::deleteItem(Q3PtrCollection::Item d)
{
    if (del_item) delete (type *)d;
}


template<class type>
class Q3AsciiCacheIterator : public Q3GCacheIterator
{
public:
    Q3AsciiCacheIterator(const Q3AsciiCache<type> &c):Q3GCacheIterator((Q3GCache &)c) {}
    Q3AsciiCacheIterator(const Q3AsciiCacheIterator<type> &ci)
				: Q3GCacheIterator((Q3GCacheIterator &)ci) {}
    Q3AsciiCacheIterator<type> &operator=(const Q3AsciiCacheIterator<type>&ci)
	{ return (Q3AsciiCacheIterator<type>&)Q3GCacheIterator::operator=(ci); }
    uint  count()   const     { return Q3GCacheIterator::count(); }
    bool  isEmpty() const     { return Q3GCacheIterator::count() == 0; }
    bool  atFirst() const     { return Q3GCacheIterator::atFirst(); }
    bool  atLast()  const     { return Q3GCacheIterator::atLast(); }
    type *toFirst()	      { return (type *)Q3GCacheIterator::toFirst(); }
    type *toLast()	      { return (type *)Q3GCacheIterator::toLast(); }
    operator type *() const   { return (type *)Q3GCacheIterator::get(); }
    type *current()   const   { return (type *)Q3GCacheIterator::get(); }
    const char *currentKey() const { return Q3GCacheIterator::getKeyAscii(); }
    type *operator()()	      { return (type *)Q3GCacheIterator::operator()();}
    type *operator++()	      { return (type *)Q3GCacheIterator::operator++(); }
    type *operator+=(uint j)  { return (type *)Q3GCacheIterator::operator+=(j);}
    type *operator--()	      { return (type *)Q3GCacheIterator::operator--(); }
    type *operator-=(uint j)  { return (type *)Q3GCacheIterator::operator-=(j);}
};

QT_END_HEADER

#endif // Q3ASCIICACHE_H
