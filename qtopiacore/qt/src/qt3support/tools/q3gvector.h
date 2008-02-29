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

#ifndef Q3GVECTOR_H
#define Q3GVECTOR_H

#include <Qt3Support/q3ptrcollection.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

class Q_COMPAT_EXPORT Q3GVector : public Q3PtrCollection	// generic vector
{
friend class Q3GList;				// needed by Q3GList::toVector
public:
#ifndef QT_NO_DATASTREAM
    QDataStream &read( QDataStream & );		// read vector from stream
    QDataStream &write( QDataStream & ) const;	// write vector to stream
#endif
    virtual int compareItems( Item, Item );

protected:
    Q3GVector();					// create empty vector
    Q3GVector( uint size );			// create vector with nullptrs
    Q3GVector( const Q3GVector &v );		// make copy of other vector
   ~Q3GVector();

    Q3GVector &operator=( const Q3GVector &v );	// assign from other vector
    bool operator==( const Q3GVector &v ) const;

    Item	 *data()    const	{ return vec; }
    uint  size()    const	{ return len; }
    uint  count()   const	{ return numItems; }

    bool  insert( uint index, Item );		// insert item at index
    bool  remove( uint index );			// remove item
    Item	  take( uint index );			// take out item

    void  clear();				// clear vector
    bool  resize( uint newsize );		// resize vector

    bool  fill( Item, int flen );		// resize and fill vector

    void  sort();				// sort vector
    int	  bsearch( Item ) const;			// binary search (when sorted)

    int	  findRef( Item, uint index ) const;	// find exact item in vector
    int	  find( Item, uint index ) const;	// find equal item in vector
    uint  containsRef( Item ) const;		// get number of exact matches
    uint  contains( Item ) const;		// get number of equal matches

    Item	  at( uint index ) const		// return indexed item
    {
#if defined(QT_CHECK_RANGE)
	if ( index >= len )
	    warningIndexRange( index );
#endif
	return vec[index];
    }

    bool insertExpand( uint index, Item );	// insert, expand if necessary

    void toList( Q3GList * ) const;		// put items in list

#ifndef QT_NO_DATASTREAM
    virtual QDataStream &read( QDataStream &, Item & );
    virtual QDataStream &write( QDataStream &, Item ) const;
#endif
private:
    Item	 *vec;
    uint  len;
    uint  numItems;

    static void warningIndexRange( uint );
};


/*****************************************************************************
  Q3GVector stream functions
 *****************************************************************************/

#ifndef QT_NO_DATASTREAM
Q_COMPAT_EXPORT QDataStream &operator>>( QDataStream &, Q3GVector & );
Q_COMPAT_EXPORT QDataStream &operator<<( QDataStream &, const Q3GVector & );
#endif

QT_END_HEADER

#endif // Q3GVECTOR_H
