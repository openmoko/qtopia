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

#ifndef QGCACHE_H
#define QGCACHE_H

#ifndef QT_H
#include "qptrcollection.h"
#include "qglist.h"
#include "qgdict.h"
#endif // QT_H


class QCList;					// internal classes
class QCListIt;
class QCDict;


class Q_EXPORT QGCache : public QPtrCollection	// generic LRU cache
{
friend class QGCacheIterator;
protected:
    enum KeyType { StringKey, AsciiKey, IntKey, PtrKey };
      // identical to QGDict's, but PtrKey is not used at the moment

    QGCache( int maxCost, uint size, KeyType kt, bool caseSensitive,
	     bool copyKeys );
    QGCache( const QGCache & );			// not allowed, calls fatal()
   ~QGCache();
    QGCache &operator=( const QGCache & );	// not allowed, calls fatal()

    uint    count()	const;
    uint    size()	const;
    int	    maxCost()	const	{ return mCost; }
    int	    totalCost() const	{ return tCost; }
    void    setMaxCost( int maxCost );
    void    clear();

    bool    insert_string( const QString &key, QPtrCollection::Item,
			   int cost, int priority );
    bool    insert_other( const char *key, QPtrCollection::Item,
			  int cost, int priority );
    bool    remove_string( const QString &key );
    bool    remove_other( const char *key );
    QPtrCollection::Item take_string( const QString &key );
    QPtrCollection::Item take_other( const char *key );

    QPtrCollection::Item find_string( const QString &key, bool ref=TRUE ) const;
    QPtrCollection::Item find_other( const char *key, bool ref=TRUE ) const;

    void    statistics() const;

private:
    bool    makeRoomFor( int cost, int priority = -1 );
    KeyType keytype;
    QCList *lruList;
    QCDict *dict;
    int	    mCost;
    int	    tCost;
    bool    copyk;
};


class Q_EXPORT QGCacheIterator			// generic cache iterator
{
protected:
    QGCacheIterator( const QGCache & );
    QGCacheIterator( const QGCacheIterator & );
   ~QGCacheIterator();
    QGCacheIterator &operator=( const QGCacheIterator & );

    uint	      count()   const;
    bool	      atFirst() const;
    bool	      atLast()  const;
    QPtrCollection::Item toFirst();
    QPtrCollection::Item toLast();

    QPtrCollection::Item get() const;
    QString	      getKeyString() const;
    const char       *getKeyAscii()  const;
    long	      getKeyInt()    const;

    QPtrCollection::Item operator()();
    QPtrCollection::Item operator++();
    QPtrCollection::Item operator+=( uint );
    QPtrCollection::Item operator--();
    QPtrCollection::Item operator-=( uint );

protected:
    QCListIt *it;				// iterator on cache list
};


#endif // QGCACHE_H
