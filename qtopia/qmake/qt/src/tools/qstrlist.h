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

#ifndef QSTRLIST_H
#define QSTRLIST_H

#ifndef QT_H
#include "qstring.h"
#include "qptrlist.h"
#include "qdatastream.h"
#endif // QT_H

#if defined(Q_QDOC)
class QStrListIterator : public QPtrListIterator<char>
{
};
#else
typedef QPtrListIterator<char> QStrListIterator;
#endif

class Q_EXPORT QStrList : public QPtrList<char>
{
public:
    QStrList( bool deepCopies=TRUE ) { dc = deepCopies; del_item = deepCopies; }
    QStrList( const QStrList & );
    ~QStrList()			{ clear(); }
    QStrList& operator=( const QStrList & );

private:
    QPtrCollection::Item newItem( QPtrCollection::Item d ) { return dc ? qstrdup( (const char*)d ) : d; }
    void deleteItem( QPtrCollection::Item d ) { if ( del_item ) delete[] (char*)d; }
    int compareItems( QPtrCollection::Item s1, QPtrCollection::Item s2 ) { return qstrcmp((const char*)s1,
							 (const char*)s2); }
#ifndef QT_NO_DATASTREAM
    QDataStream &read( QDataStream &s, QPtrCollection::Item &d )
				{ s >> (char *&)d; return s; }
    QDataStream &write( QDataStream &s, QPtrCollection::Item d ) const
				{ return s << (const char *)d; }
#endif
    bool  dc;
};


class Q_EXPORT QStrIList : public QStrList	// case insensitive string list
{
public:
    QStrIList( bool deepCopies=TRUE ) : QStrList( deepCopies ) {}
    ~QStrIList()			{ clear(); }
private:
    int	  compareItems( QPtrCollection::Item s1, QPtrCollection::Item s2 )
				{ return qstricmp((const char*)s1,
						    (const char*)s2); }
};


inline QStrList & QStrList::operator=( const QStrList &strList )
{
    clear();
    dc = strList.dc;
    del_item = dc;
    QPtrList<char>::operator=( strList );
    return *this;
}

inline QStrList::QStrList( const QStrList &strList )
    : QPtrList<char>( strList )
{
    dc = FALSE;
    operator=( strList );
}

#endif // QSTRLIST_H
