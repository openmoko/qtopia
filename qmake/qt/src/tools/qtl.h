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

#ifndef QTL_H
#define QTL_H

#ifndef QT_H
#include "qglobal.h"
#include "qtextstream.h"
#include "qstring.h"
#endif // QT_H

#ifndef QT_NO_TEXTSTREAM
template <class T>
class QTextOStreamIterator
{
protected:
    QTextOStream& stream;
    QString separator;

public:
    QTextOStreamIterator( QTextOStream& s) : stream( s ) {}
    QTextOStreamIterator( QTextOStream& s, const QString& sep )
	: stream( s ), separator( sep )  {}
    QTextOStreamIterator<T>& operator= ( const T& x ) {
	stream << x;
	if ( !separator.isEmpty() )
	    stream << separator;
	return *this;
    }
    QTextOStreamIterator<T>& operator*() { return *this; }
    QTextOStreamIterator<T>& operator++() { return *this; }
    QTextOStreamIterator<T>& operator++(int) { return *this; }
};
#endif //QT_NO_TEXTSTREAM

template <class InputIterator, class OutputIterator>
inline OutputIterator qCopy( InputIterator _begin, InputIterator _end,
			     OutputIterator _dest )
{
    while( _begin != _end )
	*_dest++ = *_begin++;
    return _dest;
}

template <class BiIterator, class BiOutputIterator>
inline BiOutputIterator qCopyBackward( BiIterator _begin, BiIterator _end,
				       BiOutputIterator _dest )
{
    while ( _begin != _end )
	*--_dest = *--_end;
    return _dest;
}

template <class InputIterator1, class InputIterator2>
inline bool qEqual( InputIterator1 first1, InputIterator1 last1, InputIterator2 first2 )
{
    // ### compare using !(*first1 == *first2) in Qt 4.0
    for ( ; first1 != last1; ++first1, ++first2 )
	if ( *first1 != *first2 )
	    return FALSE;
    return TRUE;
}

template <class ForwardIterator, class T>
inline void qFill( ForwardIterator first, ForwardIterator last, const T& val )
{
    for ( ; first != last; ++first )
	*first = val;
}

#if 0
template <class BiIterator, class OutputIterator>
inline OutputIterator qReverseCopy( BiIterator _begin, BiIterator _end,
				    OutputIterator _dest )
{
    while ( _begin != _end ) {
	--_end;
	*_dest = *_end;
	++_dest;
    }
    return _dest;
}
#endif


template <class InputIterator, class T>
inline InputIterator qFind( InputIterator first, InputIterator last,
			    const T& val )
{
    while ( first != last && *first != val )
	++first;
    return first;
}

template <class InputIterator, class T, class Size>
inline void qCount( InputIterator first, InputIterator last, const T& value,
		    Size& n )
{
    for ( ; first != last; ++first )
	if ( *first == value )
	    ++n;
}

template <class T>
inline void qSwap( T& _value1, T& _value2 )
{
    T tmp = _value1;
    _value1 = _value2;
    _value2 = tmp;
}


template <class InputIterator>
Q_INLINE_TEMPLATES void qBubbleSort( InputIterator b, InputIterator e )
{
    // Goto last element;
    InputIterator last = e;
    --last;
    // only one element or no elements ?
    if ( last == b )
	return;

    // So we have at least two elements in here
    while( b != last ) {
	bool swapped = FALSE;
	InputIterator swap_pos = b;
	InputIterator x = e;
	InputIterator y = x;
	y--;
	do {
	    --x;
	    --y;
	    if ( *x < *y ) {
		swapped = TRUE;
		qSwap( *x, *y );
		swap_pos = y;
	    }
	} while( y != b );
	if ( !swapped )
	    return;
	b = swap_pos;
	b++;
    }
}


template <class Container>
inline void qBubbleSort( Container &c )
{
  qBubbleSort( c.begin(), c.end() );
}


template <class Value>
Q_INLINE_TEMPLATES void qHeapSortPushDown( Value* heap, int first, int last )
{
    int r = first;
    while ( r <= last / 2 ) {
	if ( last == 2 * r ) {
	    // node r has only one child
	    if ( heap[2 * r] < heap[r] )
		qSwap( heap[r], heap[2 * r] );
	    r = last;
	} else {
	    // node r has two children
	    if ( heap[2 * r] < heap[r] && !(heap[2 * r + 1] < heap[2 * r]) ) {
		// swap with left child
		qSwap( heap[r], heap[2 * r] );
		r *= 2;
	    } else if ( heap[2 * r + 1] < heap[r]
			&& heap[2 * r + 1] < heap[2 * r] ) {
		// swap with right child
		qSwap( heap[r], heap[2 * r + 1] );
		r = 2 * r + 1;
	    } else {
		r = last;
	    }
	}
    }
}


template <class InputIterator, class Value>
Q_INLINE_TEMPLATES void qHeapSortHelper( InputIterator b, InputIterator e, Value, uint n )
{
    // Create the heap
    InputIterator insert = b;
    Value* realheap = new Value[n];
    // Wow, what a fake. But I want the heap to be indexed as 1...n
    Value* heap = realheap - 1;
    int size = 0;
    for( ; insert != e; ++insert ) {
	heap[++size] = *insert;
	int i = size;
	while( i > 1 && heap[i] < heap[i / 2] ) {
	    qSwap( heap[i], heap[i / 2] );
	    i /= 2;
	}
    }

    // Now do the sorting
    for( uint i = n; i > 0; i-- ) {
	*b++ = heap[1];
	if ( i > 1 ) {
	    heap[1] = heap[i];
	    qHeapSortPushDown( heap, 1, (int)i - 1 );
	}
    }

    delete[] realheap;
}


template <class InputIterator>
Q_INLINE_TEMPLATES void qHeapSort( InputIterator b, InputIterator e )
{
    // Empty ?
    if ( b == e )
	return;

    // How many entries have to be sorted ?
    InputIterator it = b;
    uint n = 0;
    while ( it != e ) {
	++n;
	++it;
    }

    // The second last parameter is a hack to retrieve the value type
    // Do the real sorting here
    qHeapSortHelper( b, e, *b, n );
}


template <class Container>
Q_INLINE_TEMPLATES void qHeapSort( Container &c )
{
    if ( c.begin() == c.end() )
	return;

    // The second last parameter is a hack to retrieve the value type
    // Do the real sorting here
    qHeapSortHelper( c.begin(), c.end(), *(c.begin()), (uint)c.count() );
}

template <class Container>
class QBackInsertIterator
{
public:
    Q_EXPLICIT QBackInsertIterator( Container &c )
	: container( &c )
    {
    }

    QBackInsertIterator<Container>&
    operator=( const Q_TYPENAME Container::value_type &value )
    {
	container->push_back( value );
	return *this;
    }

    QBackInsertIterator<Container>& operator*()
    {
	return *this;
    }

    QBackInsertIterator<Container>& operator++()
    {
	return *this;
    }

    QBackInsertIterator<Container>& operator++(int)
    {
	return *this;
    }

protected:
    Container *container;
};

template <class Container>
inline QBackInsertIterator<Container> qBackInserter( Container &c )
{
    return QBackInsertIterator<Container>( c );
}

#endif
