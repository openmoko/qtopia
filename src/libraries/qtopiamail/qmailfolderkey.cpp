/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include "qmailfolderkey.h"
#include "qmailfolderkey_p.h"

/*!
    \class QMailFolderKey
    \mainclass
    \preliminary
    \brief The QMailFolderKey class defines the parameters used for querying a subset of 
    all available mail folders from the mail store.
    \ingroup messaginglibrary

    A QMailFolderKey is composed of a folder property, an optional comparison operator
    and a comparison value. The QMailFolderKey class is used in conjunction with the QMailStore::query() 
    and QMailStore::count() functions to filter results which meet the criteria defined
    by the key.

    QMailFolderKey's can be combined using the logical operators (&), (|) and (~) to 
    build more sophisticated queries.
    
    For example:
    To create a query for all folders named "inbox" or "sms"
    \code
    QMailFolderKey inboxKey(QMailFolderKey::Name,"inbox");
    QMailFolderKey smsKey(QMailFolderKey::Name,"sms");
    QMailIdList results = QMailStore::query(inboxKey | smsKey);
    \endcode
    
    To query all subfolders with name "foo" for a given folder \a parent:
    \code
    \\assuming parent has been retrieved from the mail store.
    QMailFolder parent; 
    QMailFolderKey nameKey(QMailFolderKey::Name,"foo");
    QMailFolderKey childKey(QMailFolderKey::ParentId,parent.id());
    QMailIdList results = QMailStore::query(nameKey & childKey);
    \endcode 
    
    \sa QMailStore, QMailMessageKey
*/

/*!
    \enum QMailFolderKey::Operand

    Defines the comparison operators that can be used to comapare QMailFolder::Property elements with 
    user sepecified values.

    \value LessThan represents the '<' operator.
    \value LessThanEqual represents the '<=' operator.
    \value GreaterThan represents the '>' operator.
    \value GreaterThanEqual represents the '>= operator'.
    \value Equal represents the '=' operator.
    \value NotEqual represents the '!=' operator.
    \value Contains represents an operation in which an associated property is checked to see if it
    contains a provided value. For most property types this will perform a string based check. For
    Flag type properties this will perform a check to see if a flag bit value is set.
*/

/*!
    \enum QMailFolderKey::Property

    This enum type describes the queryable data properties of a QMailFolder.

    \value Id The ID of the folder.
    \value Name The name of the folder.
    \value ParentId the ID of the parent folder for a given folder.
*/



/*!
    Construct a QMailFolderKey which defines a query parameter where
    QMailFolder::Property \a p is compared using comparison operator
    \a c with a value \a value.
*/

QMailFolderKey::QMailFolderKey(const Property& p, const QVariant& value, const Operand& c)
{
    init(p,c,value);
}

/*!
    Create a copy of the QMailFolderKey \a other.
*/

QMailFolderKey::QMailFolderKey(const QMailFolderKey& other)
{
    d = other.d;
}


/*!
    Destroys this QMailFolderKey.
*/


QMailFolderKey::~QMailFolderKey()
{
}

/*!
    Returns a key that is the logical NOT of the value of this key.
*/

QMailFolderKey QMailFolderKey::operator~() const
{
    QMailFolderKey k(*this);
    k.d->negated = !d->negated;
    return k;
}

/*!
    Returns a key that is the logical AND of this key and the value of key \a other.
*/

QMailFolderKey QMailFolderKey::operator&(const QMailFolderKey& other) const
{
    QMailFolderKey k;
    k.d->negated = false;
    k.d->logicalOp = QMailFolderKeyPrivate::And;

    if(d->logicalOp != QMailFolderKeyPrivate::Or && !d->negated && other.d->logicalOp != QMailFolderKeyPrivate::Or && !other.d->negated)
    {
        k.d->subKeys = d->subKeys + other.d->subKeys;
        k.d->arguments = d->arguments + other.d->arguments;
    }
    else
    {
        k.d->subKeys.append(*this);
        k.d->subKeys.append(other); 
    }
    return k;            
}

/*!
    Returns a key that is the logical OR of this key and the value of key \a other.
*/

QMailFolderKey QMailFolderKey::operator|(const QMailFolderKey& other) const
{
    QMailFolderKey k;
    k.d->negated = false;
    k.d->logicalOp = QMailFolderKeyPrivate::Or;
    if(d->logicalOp != QMailFolderKeyPrivate::And && 
       !d->negated && 
       other.d->logicalOp != QMailFolderKeyPrivate::And && 
       !other.d->negated)
    {
        k.d->subKeys = d->subKeys + other.d->subKeys;
        k.d->arguments = d->arguments + other.d->arguments;
    }
    else
    {
        k.d->subKeys.append(*this);    
        k.d->subKeys.append(other);
    }

    return k;
}

/*!
    Performs a logical AND with this key and the key \a other and assigns the result
    to this key.
*/

QMailFolderKey& QMailFolderKey::operator&=(const QMailFolderKey& other)
{
    *this = *this & other;
    return *this;
}

/*!
    Performs a logical OR with this key and the key \a other and assigns the result
    to this key.
*/

QMailFolderKey& QMailFolderKey::operator|=(const QMailFolderKey& other) 
{
    *this = *this | other;
    return *this;
}

/*!
    Returns \c true if the value of this key is the same as the key \a other. Returns 
    \c false otherwise.
*/

bool QMailFolderKey::operator==(const QMailFolderKey& other) const
{
    return d->negated == other.d->negated &&
           d->logicalOp == other.d->logicalOp &&
           d->subKeys == other.d->subKeys && 
           d->arguments == other.d->arguments;
}

/*!
    Returns \c true if the value of this key is not the same as the key \a other. Returns
    \c false otherwise.
*/

bool QMailFolderKey::operator!=(const QMailFolderKey& other) const
{
   return !(*this == other); 
}

/*!
    Assign the value of the QMailFolderKey \a other to this.
*/

QMailFolderKey& QMailFolderKey::operator=(const QMailFolderKey& other)
{
    d = other.d;
    return *this;
}


/*!
    Create a QMailFolderKey with default params.
*/

QMailFolderKey::QMailFolderKey()
{
    d = new QMailFolderKeyPrivate();
    d->negated = false;
    d->logicalOp = QMailFolderKeyPrivate::None;
}

/*!
    Initialize the key class.
*/

void QMailFolderKey::init(const Property& p , const Operand& op, const QVariant& value)
{
    d = new QMailFolderKeyPrivate();
    d->logicalOp = QMailFolderKeyPrivate::None;
    d->negated = false;
    QMailFolderKeyPrivate::Argument m;
    m.property = p;
    m.op = op;
    m.value = value;
    d->arguments.append(m);
}

