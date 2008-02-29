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
