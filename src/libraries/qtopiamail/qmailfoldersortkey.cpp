/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qmailfoldersortkey.h"
#include "qmailfoldersortkey_p.h"


/*!
    \class QMailFolderSortKey
    \mainclass
    \preliminary
    \brief The QMailFolderSortKey class defines the parameters used for sorting a subset of
    queried folders from the mail store.
    \ingroup messaginglibrary

    A QMailFolderSortKey is composed of a folder property to sort and a sort order.
    The QMailFolderSortKey class is used in conjunction with the QMailStore::queryFolders()
    function to sort folder results according to the criteria defined by the sort key.

    For example:
    To create a query for all folders sorted by the name in ascending order:
    \code
    QMailFolderSortKey sortNameKey(QMailFolderSortKey::Name,Qt::Ascending);
    QMailIdList results = QMailStore::instance()->queryFolders(sortNameKey);
    \endcode

    \sa QMailStore, QMailFolderKey
*/

/*!
    \enum QMailFolderSortKey::Property

    This enum type describes the sortable data properties of a QMailFolder.

    \value Id The ID of the folder.
    \value Name The name of the folder.
    \value ParentId the ID of the parent folder for a given folder.
*/

/*!
    Create a QMailFolderSortKey with specifying matching parameters.

    A default-constructed key (one for which isEmpty() returns true) sorts no folders.

    The result of combining an empty key with a non-empty key is the same as the original
    non-empty key.

    The result of combining two empty keys is an empty key.
*/

QMailFolderSortKey::QMailFolderSortKey()
{
	d = new QMailFolderSortKeyPrivate();
}

/*!
    Construct a QMailFolderSortKey which sorts a set of results based on the
    QMailFolderSortKey::Property \a p and the Qt::SortOrder \a order
*/

QMailFolderSortKey::QMailFolderSortKey(const Property& p, const Qt::SortOrder& order)
{
    d = new QMailFolderSortKeyPrivate();
    QMailFolderSortKeyPrivate::Argument a(p,order);
    d->arguments.append(a);
}

/*!
    Create a copy of the QMailFolderSortKey \a other.
*/

QMailFolderSortKey::QMailFolderSortKey(const QMailFolderSortKey& other)
{
    d = other.d;
}


/*!
    Destroys this QMailFolderSortKey.
*/


QMailFolderSortKey::~QMailFolderSortKey()
{
}

/*!
    Returns a key that is the logical AND of this key and the value of key \a other.
*/

QMailFolderSortKey QMailFolderSortKey::operator&(const QMailFolderSortKey& other) const
{
    QMailFolderSortKey k;
    k.d->arguments = d->arguments + other.d->arguments;
    return k;
}

/*!
    Performs a logical AND with this key and the key \a other and assigns the result
    to this key.
*/

QMailFolderSortKey& QMailFolderSortKey::operator&=(const QMailFolderSortKey& other)
{
    *this = *this & other;
    return *this;
}

/*!
    Returns \c true if the value of this key is the same as the key \a other. Returns
    \c false otherwise.
*/

bool QMailFolderSortKey::operator==(const QMailFolderSortKey& other) const
{
    return d->arguments == other.d->arguments;
}
/*!
    Returns \c true if the value of this key is not the same as the key \a other. Returns
    \c false otherwise.
*/

bool QMailFolderSortKey::operator!=(const QMailFolderSortKey& other) const
{
   return !(*this == other);
}

/*!
    Assign the value of the QMailFolderSortKey \a other to this.
*/

QMailFolderSortKey& QMailFolderSortKey::operator=(const QMailFolderSortKey& other)
{
    d = other.d;
    return *this;
}

/*!
    Returns true if the key remains empty after default construction; otherwise returns false.
*/

bool QMailFolderSortKey::isEmpty() const
{
    return d->arguments.isEmpty();
}

