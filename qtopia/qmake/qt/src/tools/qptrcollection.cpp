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

#include "qptrcollection.h"

/*!
    \class QPtrCollection qptrcollection.h
    \reentrant
    \brief The QPtrCollection class is the base class of most pointer-based Qt collections.

    \ingroup collection
    \ingroup tools

    The QPtrCollection class is an abstract base class for the Qt
    \link collection.html collection classes\endlink QDict, QPtrList,
    etc. Qt also includes value based collections, e.g. QValueList,
    QMap, etc.

    A QPtrCollection only knows about the number of objects in the
    collection and the deletion strategy (see setAutoDelete()).

    A collection is implemented using the \c Item (generic collection
    item) type, which is a \c void*. The template classes that create
    the real collections cast the \c Item to the required type.
*/


/*!
    \enum QPtrCollection::Item

    This type is the generic "item" in a QPtrCollection.
*/


/*!
    \fn QPtrCollection::QPtrCollection()

    Constructs a collection. The constructor is protected because
    QPtrCollection is an abstract class.
*/

/*!
    \fn QPtrCollection::QPtrCollection( const QPtrCollection & source )

    Constructs a copy of \a source with autoDelete() set to FALSE. The
    constructor is protected because QPtrCollection is an abstract
    class.

    Note that if \a source has autoDelete turned on, copying it will
    risk memory leaks, reading freed memory, or both.
*/

/*!
    \fn QPtrCollection::~QPtrCollection()

    Destroys the collection. The destructor is protected because
    QPtrCollection is an abstract class.
*/


/*!
    \fn bool QPtrCollection::autoDelete() const

    Returns the setting of the auto-delete option. The default is FALSE.

    \sa setAutoDelete()
*/

/*!
    \fn void QPtrCollection::setAutoDelete( bool enable )

    Sets the collection to auto-delete its contents if \a enable is
    TRUE and to never delete them if \a enable is FALSE.

    If auto-deleting is turned on, all the items in a collection are
    deleted when the collection itself is deleted. This is convenient
    if the collection has the only pointer to the items.

    The default setting is FALSE, for safety. If you turn it on, be
    careful about copying the collection - you might find yourself
    with two collections deleting the same items.

    Note that the auto-delete setting may also affect other functions
    in subclasses. For example, a subclass that has a remove()
    function will remove the item from its data structure, and if
    auto-delete is enabled, will also delete the item.

    \sa autoDelete()
*/


/*!
    \fn virtual uint QPtrCollection::count() const

    Returns the number of objects in the collection.
*/

/*!
    \fn virtual void QPtrCollection::clear()

    Removes all objects from the collection. The objects will be
    deleted if auto-delete has been enabled.

    \sa setAutoDelete()
*/

/*!
    \fn void QPtrCollection::deleteItem( Item d )

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

QPtrCollection::Item QPtrCollection::newItem( Item d )
{
    return d;					// just return reference
}
