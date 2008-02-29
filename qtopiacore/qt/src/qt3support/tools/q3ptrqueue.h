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

#ifndef Q3PTRQUEUE_H
#define Q3PTRQUEUE_H

#include <Qt3Support/q3glist.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

template<class type>
class Q3PtrQueue : protected Q3GList
{
public:
    Q3PtrQueue()				{}
    Q3PtrQueue( const Q3PtrQueue<type> &q ) : Q3GList(q) {}
    ~Q3PtrQueue()			{ clear(); }
    Q3PtrQueue<type>& operator=(const Q3PtrQueue<type> &q)
			{ return (Q3PtrQueue<type>&)Q3GList::operator=(q); }
    bool  autoDelete() const		{ return Q3PtrCollection::autoDelete(); }
    void  setAutoDelete( bool del )	{ Q3PtrCollection::setAutoDelete(del); }
    uint  count()   const		{ return Q3GList::count(); }
    bool  isEmpty() const		{ return Q3GList::count() == 0; }
    void  enqueue( const type *d )	{ Q3GList::append(Item(d)); }
    type *dequeue()			{ return (type *)Q3GList::takeFirst();}
    bool  remove()			{ return Q3GList::removeFirst(); }
    void  clear()			{ Q3GList::clear(); }
    type *head()    const		{ return (type *)Q3GList::cfirst(); }
	  operator type *() const	{ return (type *)Q3GList::cfirst(); }
    type *current() const		{ return (type *)Q3GList::cfirst(); }

#ifdef qdoc
protected:
    virtual QDataStream& read( QDataStream&, Q3PtrCollection::Item& );
    virtual QDataStream& write( QDataStream&, Q3PtrCollection::Item ) const;
#endif

private:
    void  deleteItem( Item d );
};

#if !defined(Q_BROKEN_TEMPLATE_SPECIALIZATION)
template<> inline void Q3PtrQueue<void>::deleteItem( Q3PtrCollection::Item )
{
}
#endif

template<class type> inline void Q3PtrQueue<type>::deleteItem( Q3PtrCollection::Item d )
{
    if ( del_item ) delete (type *)d;
}

QT_END_HEADER

#endif // Q3PTRQUEUE_H
