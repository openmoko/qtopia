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

#ifndef QPAIR_H
#define QPAIR_H

#ifndef QT_H
#include "qglobal.h"
#include "qdatastream.h"
#endif // QT_H

template <class T1, class T2>
struct QPair
{
    typedef T1 first_type;
    typedef T2 second_type;

    QPair()
	: first( T1() ), second( T2() )
    {}
    QPair( const T1& t1, const T2& t2 )
	: first( t1 ), second( t2 )
    {}

    T1 first;
    T2 second;
};

template <class T1, class T2>
Q_INLINE_TEMPLATES bool operator==( const QPair<T1, T2>& x, const QPair<T1, T2>& y )
{
    return x.first == y.first && x.second == y.second;
}

template <class T1, class T2>
Q_INLINE_TEMPLATES bool operator<( const QPair<T1, T2>& x, const QPair<T1, T2>& y )
{
    return x.first < y.first ||
	   ( !( y.first < x.first ) && x.second < y.second );
}

template <class T1, class T2>
Q_INLINE_TEMPLATES QPair<T1, T2> qMakePair( const T1& x, const T2& y )
{
    return QPair<T1, T2>( x, y );
}

#ifndef QT_NO_DATASTREAM
template <class T1, class T2>
inline QDataStream& operator>>( QDataStream& s, QPair<T1, T2>& p )
{
    s >> p.first >> p.second;
    return s;
}

template <class T1, class T2>
inline QDataStream& operator<<( QDataStream& s, const QPair<T1, T2>& p )
{
    s << p.first << p.second;
    return s;
}
#endif

#endif
