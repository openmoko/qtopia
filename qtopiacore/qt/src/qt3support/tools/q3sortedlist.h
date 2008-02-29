/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef Q3SORTEDLIST_H
#define Q3SORTEDLIST_H

#include <Qt3Support/q3ptrlist.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

template<class type>
class Q3SortedList : public Q3PtrList<type>
{
public:
    Q3SortedList() {}
    Q3SortedList( const Q3SortedList<type> &l ) : Q3PtrList<type>(l) {}
    ~Q3SortedList() { this->clear(); }
    Q3SortedList<type> &operator=(const Q3SortedList<type> &l)
      { return (Q3SortedList<type>&)Q3PtrList<type>::operator=(l); }

    virtual int compareItems( Q3PtrCollection::Item s1, Q3PtrCollection::Item s2 )
      { if ( *((type*)s1) == *((type*)s2) ) return 0; return ( *((type*)s1) < *((type*)s2) ? -1 : 1 ); }
};

QT_END_HEADER

#endif // Q3SORTEDLIST_H
