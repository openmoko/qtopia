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

#include "q3ptrcollection.h"

/*!
    \class Q3PtrCollection qptrcollection.h
    \reentrant
    \brief The Q3PtrCollection class is the base class of most pointer-based Qt collections.

    \compat

    The Q3PtrCollection class is an abstract base class for the Qt
    collection classes QDict, Q3PtrList,
    etc.

    A Q3PtrCollection only knows about the number of objects in the
    collection and the deletion strategy (see setAutoDelete()).

    A collection is implemented using the \c Item (generic collection
    item) type, which is a \c void*. The template classes that create
    the real collections cast the \c Item to the required type.
*/


/*!
    \typedef Q3PtrCollection::Item

    This type is the generic "item" in a Q3PtrCollection.
*/


/*!
    \fn Q3PtrCollection::Q3PtrCollection()

    Constructs a collection. The constructor is protected because
    Q3PtrCollection is an abstract class.
*/

/*!
    \fn Q3PtrCollection::Q3PtrCollection( const Q3PtrCollection & source )

    Constructs a copy of \a source with autoDelete() set to false. The
    constructor is protected because Q3PtrCollection is an abstract
    class.

    Note that if \a source has autoDelete turned on, copying it will
    risk memory leaks, reading freed memory, or both.
*/

/*!
    \fn Q3PtrCollection::~Q3PtrCollection()

    Destroys the collection. The destructor is protected because
    Q3PtrCollection is an abstract class.
*/


/*!
    \fn bool Q3PtrCollection::autoDelete() const

    Returns the setting of the auto-delete option. The default is false.

    \sa setAutoDelete()
*/

/*!
    \fn void Q3PtrCollection::setAutoDelete( bool enable )

    Sets the collection to auto-delete its contents if \a enable is
    true and to never delete them if \a enable is false.

    If auto-deleting is turned on, all the items in a collection are
    deleted when the collection itself is deleted. This is convenient
    if the collection has the only pointer to the items.

    The default setting is false, for safety. If you turn it on, be
    careful about copying the collection - you might find yourself
    with two collections deleting the same items.

    Note that the auto-delete setting may also affect other functions
    in subclasses. For example, a subclass that has a remove()
    function will remove the item from its data structure, and if
    auto-delete is enabled, will also delete the item.

    \sa autoDelete()
*/


/*!
    \fn virtual uint Q3PtrCollection::count() const

    Returns the number of objects in the collection.
*/

/*!
    \fn virtual void Q3PtrCollection::clear()

    Removes all objects from the collection. The objects will be
    deleted if auto-delete has been enabled.

    \sa setAutoDelete()
*/

/*!
    \fn void Q3PtrCollection::deleteItem( Item d )

    Reimplement this function if you want to be able to delete items.

    Deletes an item that is about to be removed from the collection.

    This function has to reimplemented in the collection template
    classes, and should \e only delete item \a d if auto-delete has
    been enabled.

    \warning If you reimplement this function you must also
    reimplement the destructor and call the virtual function clear()
    from your destructor. This is due to the way virtual functions and
    destructors work in C++: Virtual functions in derived classes
    cannot be called from a destructor. If you do not do this, your
    deleteItem() function will not be called when the container is
    destroyed.

    \sa newItem(), setAutoDelete()
*/

/*!
    Virtual function that creates a copy of an object that is about to
    be inserted into the collection.

    The default implementation returns the \a d pointer, i.e. no copy
    is made.

    This function is seldom reimplemented in the collection template
    classes. It is not common practice to make a copy of something
    that is being inserted.

    \sa deleteItem()
*/

Q3PtrCollection::Item Q3PtrCollection::newItem(Item d)
{
    return d;					// just return reference
}
