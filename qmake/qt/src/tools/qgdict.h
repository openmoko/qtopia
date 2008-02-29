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

#ifndef QGDICT_H
#define QGDICT_H

#ifndef QT_H
#include "qptrcollection.h"
#include "qstring.h"
#endif // QT_H

class QGDictIterator;
class QGDItList;


class QBaseBucket				// internal dict node
{
public:
    QPtrCollection::Item	 getData()			{ return data; }
    QPtrCollection::Item	 setData( QPtrCollection::Item d ) { return data = d; }
    QBaseBucket		*getNext()			{ return next; }
    void		 setNext( QBaseBucket *n)	{ next = n; }
protected:
    QBaseBucket( QPtrCollection::Item d, QBaseBucket *n ) : data(d), next(n) {}
    QPtrCollection::Item	 data;
    QBaseBucket		*next;
};

class QStringBucket : public QBaseBucket
{
public:
    QStringBucket( const QString &k, QPtrCollection::Item d, QBaseBucket *n )
	: QBaseBucket(d,n), key(k)		{}
    const QString  &getKey() const		{ return key; }
private:
    QString	    key;
};

class QAsciiBucket : public QBaseBucket
{
public:
    QAsciiBucket( const char *k, QPtrCollection::Item d, QBaseBucket *n )
	: QBaseBucket(d,n), key(k) {}
    const char *getKey() const { return key; }
private:
    const char *key;
};

class QIntBucket : public QBaseBucket
{
public:
    QIntBucket( long k, QPtrCollection::Item d, QBaseBucket *n )
	: QBaseBucket(d,n), key(k) {}
    long  getKey() const { return key; }
private:
    long  key;
};

class QPtrBucket : public QBaseBucket
{
public:
    QPtrBucket( void *k, QPtrCollection::Item d, QBaseBucket *n )
	: QBaseBucket(d,n), key(k) {}
    void *getKey() const { return key; }
private:
    void *key;
};


class Q_EXPORT QGDict : public QPtrCollection	// generic dictionary class
{
public:
    uint	count() const	{ return numItems; }
    uint	size()	const	{ return vlen; }
    QPtrCollection::Item look_string( const QString& key, QPtrCollection::Item,
				   int );
    QPtrCollection::Item look_ascii( const char *key, QPtrCollection::Item, int );
    QPtrCollection::Item look_int( long key, QPtrCollection::Item, int );
    QPtrCollection::Item look_ptr( void *key, QPtrCollection::Item, int );
#ifndef QT_NO_DATASTREAM
    QDataStream &read( QDataStream & );
    QDataStream &write( QDataStream & ) const;
#endif
protected:
    enum KeyType { StringKey, AsciiKey, IntKey, PtrKey };

    QGDict( uint len, KeyType kt, bool cs, bool ck );
    QGDict( const QGDict & );
   ~QGDict();

    QGDict     &operator=( const QGDict & );

    bool	remove_string( const QString &key, QPtrCollection::Item item=0 );
    bool	remove_ascii( const char *key, QPtrCollection::Item item=0 );
    bool	remove_int( long key, QPtrCollection::Item item=0 );
    bool	remove_ptr( void *key, QPtrCollection::Item item=0 );
    QPtrCollection::Item take_string( const QString &key );
    QPtrCollection::Item take_ascii( const char *key );
    QPtrCollection::Item take_int( long key );
    QPtrCollection::Item take_ptr( void *key );

    void	clear();
    void	resize( uint );

    int		hashKeyString( const QString & );
    int		hashKeyAscii( const char * );

    void	statistics() const;

#ifndef QT_NO_DATASTREAM
    virtual QDataStream &read( QDataStream &, QPtrCollection::Item & );
    virtual QDataStream &write( QDataStream &, QPtrCollection::Item ) const;
#endif
private:
    QBaseBucket **vec;
    uint	vlen;
    uint	numItems;
    uint	keytype	: 2;
    uint	cases	: 1;
    uint	copyk	: 1;
    QGDItList  *iterators;
    void	   unlink_common( int, QBaseBucket *, QBaseBucket * );
    QStringBucket *unlink_string( const QString &,
				  QPtrCollection::Item item = 0 );
    QAsciiBucket  *unlink_ascii( const char *, QPtrCollection::Item item = 0 );
    QIntBucket    *unlink_int( long, QPtrCollection::Item item = 0 );
    QPtrBucket    *unlink_ptr( void *, QPtrCollection::Item item = 0 );
    void	init( uint, KeyType, bool, bool );
    friend class QGDictIterator;
};


class Q_EXPORT QGDictIterator			// generic dictionary iterator
{
friend class QGDict;
public:
    QGDictIterator( const QGDict & );
    QGDictIterator( const QGDictIterator & );
    QGDictIterator &operator=( const QGDictIterator & );
   ~QGDictIterator();

    QPtrCollection::Item toFirst();

    QPtrCollection::Item get()	     const;
    QString	      getKeyString() const;
    const char	     *getKeyAscii()  const;
    long	      getKeyInt()    const;
    void	     *getKeyPtr()    const;

    QPtrCollection::Item operator()();
    QPtrCollection::Item operator++();
    QPtrCollection::Item operator+=(uint);

protected:
    QGDict	     *dict;

private:
    QBaseBucket      *curNode;
    uint	      curIndex;
};

inline QPtrCollection::Item QGDictIterator::get() const
{
    return curNode ? curNode->getData() : 0;
}

inline QString QGDictIterator::getKeyString() const
{
    return curNode ? ((QStringBucket*)curNode)->getKey() : QString::null;
}

inline const char *QGDictIterator::getKeyAscii() const
{
    return curNode ? ((QAsciiBucket*)curNode)->getKey() : 0;
}

inline long QGDictIterator::getKeyInt() const
{
    return curNode ? ((QIntBucket*)curNode)->getKey() : 0;
}

inline void *QGDictIterator::getKeyPtr() const
{
    return curNode ? ((QPtrBucket*)curNode)->getKey() : 0;
}


#endif // QGDICT_H
