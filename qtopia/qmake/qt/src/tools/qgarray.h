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

#ifndef QGARRAY_H
#define QGARRAY_H

#ifndef QT_H
#include "qshared.h"
#endif // QT_H


class Q_EXPORT QGArray					// generic array
{
friend class QBuffer;
public:
    // do not use this, even though this is public
    // ### make protected or private in Qt 4.0 beta?
    struct array_data : public QShared {	// shared array
	array_data():data(0),len(0)
#ifdef QT_QGARRAY_SPEED_OPTIM
		    ,maxl(0)
#endif
	    {}
	char *data;				// actual array data
	uint  len;
#ifdef QT_QGARRAY_SPEED_OPTIM
	uint maxl;
#endif
    };
    QGArray();
    enum Optimization { MemOptim, SpeedOptim };
protected:
    QGArray( int, int );			// dummy; does not alloc
    QGArray( int size );			// allocate 'size' bytes
    QGArray( const QGArray &a );		// shallow copy
    virtual ~QGArray();

    QGArray    &operator=( const QGArray &a ) { return assign( a ); }

    virtual void detach()	{ duplicate(*this); }

    // ### Qt 4.0: maybe provide two versions of data(), at(), etc.
    char       *data()	 const	{ return shd->data; }
    uint	nrefs()	 const	{ return shd->count; }
    uint	size()	 const	{ return shd->len; }
    bool	isEqual( const QGArray &a ) const;

    bool	resize( uint newsize, Optimization optim );
    bool	resize( uint newsize );

    bool	fill( const char *d, int len, uint sz );

    QGArray    &assign( const QGArray &a );
    QGArray    &assign( const char *d, uint len );
    QGArray    &duplicate( const QGArray &a );
    QGArray    &duplicate( const char *d, uint len );
    void	store( const char *d, uint len );

    array_data *sharedBlock()	const		{ return shd; }
    void	setSharedBlock( array_data *p ) { shd=(array_data*)p; }

    QGArray    &setRawData( const char *d, uint len );
    void	resetRawData( const char *d, uint len );

    int		find( const char *d, uint index, uint sz ) const;
    int		contains( const char *d, uint sz ) const;

    void	sort( uint sz );
    int		bsearch( const char *d, uint sz ) const;

    char       *at( uint index ) const;

    bool	setExpand( uint index, const char *d, uint sz );

protected:
    virtual array_data *newData();
    virtual void deleteData( array_data *p );

private:
    static void msg_index( uint );
    array_data *shd;
};


inline char *QGArray::at( uint index ) const
{
#if defined(QT_CHECK_RANGE)
    if ( index >= size() ) {
	msg_index( index );
	index = 0;
    }
#endif
    return &shd->data[index];
}


#endif // QGARRAY_H
