/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt3Support module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
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
