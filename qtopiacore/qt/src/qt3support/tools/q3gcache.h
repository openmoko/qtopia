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

#ifndef Q3GCACHE_H
#define Q3GCACHE_H

#include <Qt3Support/q3ptrcollection.h>
#include <Qt3Support/q3glist.h>
#include <Qt3Support/q3gdict.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

class Q3CList;					// internal classes
class Q3CListIt;
class Q3CDict;

class Q_COMPAT_EXPORT Q3GCache : public Q3PtrCollection	// generic LRU cache
{
friend class Q3GCacheIterator;
protected:
    enum KeyType { StringKey, AsciiKey, IntKey, PtrKey };
      // identical to Q3GDict's, but PtrKey is not used at the moment

    Q3GCache(int maxCost, uint size, KeyType kt, bool caseSensitive,
	     bool copyKeys);
    Q3GCache(const Q3GCache &);			// not allowed, calls fatal()
   ~Q3GCache();
    Q3GCache &operator=(const Q3GCache &);	// not allowed, calls fatal()

    uint    count()	const;
    uint    size()	const;
    int	    maxCost()	const	{ return mCost; }
    int	    totalCost() const	{ return tCost; }
    void    setMaxCost(int maxCost);
    void    clear();

    bool    insert_string(const QString &key, Q3PtrCollection::Item,
			   int cost, int priority);
    bool    insert_other(const char *key, Q3PtrCollection::Item,
			  int cost, int priority);
    bool    remove_string(const QString &key);
    bool    remove_other(const char *key);
    Q3PtrCollection::Item take_string(const QString &key);
    Q3PtrCollection::Item take_other(const char *key);

    Q3PtrCollection::Item find_string(const QString &key, bool ref=true) const;
    Q3PtrCollection::Item find_other(const char *key, bool ref=true) const;

    void    statistics() const;

private:
    bool    makeRoomFor(int cost, int priority = -1);
    KeyType keytype;
    Q3CList *lruList;
    Q3CDict *dict;
    int	    mCost;
    int	    tCost;
    bool    copyk;
};


class Q_COMPAT_EXPORT Q3GCacheIterator			// generic cache iterator
{
protected:
    Q3GCacheIterator(const Q3GCache &);
    Q3GCacheIterator(const Q3GCacheIterator &);
   ~Q3GCacheIterator();
    Q3GCacheIterator &operator=(const Q3GCacheIterator &);

    uint	      count()   const;
    bool	      atFirst() const;
    bool	      atLast()  const;
    Q3PtrCollection::Item toFirst();
    Q3PtrCollection::Item toLast();

    Q3PtrCollection::Item get() const;
    QString	      getKeyString() const;
    const char       *getKeyAscii()  const;
    long	      getKeyInt()    const;

    Q3PtrCollection::Item operator()();
    Q3PtrCollection::Item operator++();
    Q3PtrCollection::Item operator+=(uint);
    Q3PtrCollection::Item operator--();
    Q3PtrCollection::Item operator-=(uint);

protected:
    Q3CListIt *it;				// iterator on cache list
};

QT_END_HEADER

#endif // Q3GCACHE_H
