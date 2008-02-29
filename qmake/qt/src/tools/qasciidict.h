/**********************************************************************
** Copyright (C) 2000-2004 Trolltech AS.  All rights reserved.
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

#ifndef QASCIIDICT_H
#define QASCIIDICT_H

#ifndef QT_H
#include "qgdict.h"
#endif // QT_H

template<class type>
class QAsciiDict
#ifdef Q_QDOC
	: public QPtrCollection
#else
	: public QGDict
#endif
{
public:
    QAsciiDict(int size=17, bool caseSensitive=TRUE, bool copyKeys=TRUE )
	: QGDict(size,AsciiKey,caseSensitive,copyKeys) {}
    QAsciiDict( const QAsciiDict<type> &d ) : QGDict(d) {}
   ~QAsciiDict()			{ clear(); }
    QAsciiDict<type> &operator=(const QAsciiDict<type> &d)
			{ return (QAsciiDict<type>&)QGDict::operator=(d); }
    uint  count()   const		{ return QGDict::count(); }
    uint  size()    const		{ return QGDict::size(); }
    bool  isEmpty() const		{ return QGDict::count() == 0; }

    void  insert( const char *k, const type *d )
					{ QGDict::look_ascii(k,(Item)d,1); }
    void  replace( const char *k, const type *d )
					{ QGDict::look_ascii(k,(Item)d,2); }
    bool  remove( const char *k )	{ return QGDict::remove_ascii(k); }
    type *take( const char *k )		{ return (type *)QGDict::take_ascii(k); }
    type *find( const char *k ) const
		{ return (type *)((QGDict*)this)->QGDict::look_ascii(k,0,0); }
    type *operator[]( const char *k ) const
		{ return (type *)((QGDict*)this)->QGDict::look_ascii(k,0,0); }

    void  clear()			{ QGDict::clear(); }
    void  resize( uint n )		{ QGDict::resize(n); }
    void  statistics() const		{ QGDict::statistics(); }

#ifdef Q_QDOC
protected:
    virtual QDataStream& read( QDataStream &, QPtrCollection::Item & );
    virtual QDataStream& write( QDataStream &, QPtrCollection::Item ) const;
#endif

private:
    void  deleteItem( Item d );
};

#if !defined(Q_BROKEN_TEMPLATE_SPECIALIZATION)
template<> inline void QAsciiDict<void>::deleteItem( QPtrCollection::Item )
{
}
#endif

template<class type> inline void QAsciiDict<type>::deleteItem( QPtrCollection::Item d )
{
    if ( del_item ) delete (type *)d;
}

template<class type>
class QAsciiDictIterator : public QGDictIterator
{
public:
    QAsciiDictIterator(const QAsciiDict<type> &d)
	: QGDictIterator((QGDict &)d) {}
   ~QAsciiDictIterator()      {}
    uint  count()   const     { return dict->count(); }
    bool  isEmpty() const     { return dict->count() == 0; }
    type *toFirst()	      { return (type *)QGDictIterator::toFirst(); }
    operator type *() const   { return (type *)QGDictIterator::get(); }
    type   *current() const   { return (type *)QGDictIterator::get(); }
    const char *currentKey() const { return QGDictIterator::getKeyAscii(); }
    type *operator()()	      { return (type *)QGDictIterator::operator()(); }
    type *operator++()	      { return (type *)QGDictIterator::operator++(); }
    type *operator+=(uint j)  { return (type *)QGDictIterator::operator+=(j);}
};

#define Q_DEFINED_QASCIIDICT
#include "qwinexport.h"
#endif // QASCIIDICT_H
