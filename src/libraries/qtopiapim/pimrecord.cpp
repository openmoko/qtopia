/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include "pimrecord.h"
#include <qdatastream.h>

#if defined(Q_WS_WIN32)
#include <objbase.h>
#else
extern "C" {
#include <uuid/uuid.h>
#define UUID_H_INCLUDED
}
#endif

/*!
  \class PimRecord
  \module qpepim
  \ingroup qpepim
  \brief The PimRecord class is the base class for PIM data recorded in the
  Qtopia database.

  The Pim record class contains data that is common to all data used
  by the PIM applications, in particular a unique ID and a set of
  categories.

  A PimRecord's ID is returned by uid() and is set with setUid(). Each
  category is represented by an integer. A PimRecord's categories are
  returned by categories() and are set with setCategories().

  PimRecord comparisons are provided by operator==() and operator!=().
*/

/*!
  Creates an empty record.
*/
PimRecord::PimRecord() : mCategories(0)
{
    //setUid(generateUuid()); don't do this, allow access class to.
}

/*!
  Creates a clone of the record \a other.
*/
PimRecord::PimRecord( const PimRecord &other )
	: mUid(other.mUid), mCategories(other.mCategories), 
	customMap(other.customMap) 
{ }

/*!
  Destroys the record.
*/
PimRecord::~PimRecord() { }

/*!
  Sets the record to be a clone of \a other.
*/
PimRecord &PimRecord::operator=( const PimRecord &other )
{
    mUid = other.mUid;
    mCategories = other.mCategories;
    customMap = other.customMap;
    return *this;
}

/*!
  Returns TRUE if the record has the same Uid as \a other;
  otherwise returns FALSE.

  \sa operator!=()
*/
bool PimRecord::operator==( const PimRecord &other ) const
{
    return mUid == other.mUid;
}

/*!
  Returns TRUE if the record does not have the same Uid as \a
  other; otherwise returns FALSE.

  \sa operator==()
*/
bool PimRecord::operator!=( const PimRecord &other ) const
{
    return mUid != other.mUid;
}

/*!
  \fn void PimRecord::setCategories( const QArray<int> &categories )

  Sets the record to belong to the set of categories specificed
  by \a categories.

  \sa categories()
*/

/*!
  \overload void PimRecord::setCategories( int id )

  Sets the record to belong only to the category specified by \a id.

  \sa categories()
*/

/*!
  \fn QArray<int> PimRecord::categories() const

  Returns the set of categories the record belongs to.

  \sa setCategories()
*/

/*!
  \fn QUuid PimRecord::uid() const

  Returns the unique ID for this record.
*/

/*!
  \fn QUuid PimRecord::p_uid() const
  \internal

  Returns the unique ID for this record.
*/

/*!
  \fn void PimRecord::p_setUid(QUuid)
  \internal

  Sets the record to have unique ID \a uid.
*/

/*!
  Returns a new Universally Unique Identifier.
*/
QUuid PimRecord::generateUuid()
{
#if defined(Q_WS_WIN32)
    GUID guid;
    HRESULT h = CoCreateGuid( &guid );
    if ( h != S_OK ) {
	qWarning(" could not create a Uuid" );
	return QUuid();
    }
    return QUuid( guid );
#else
 //   uuid_t uuid;
  //  uuid_generate( uuid );

   // return QUuid( uuid );
    // a uuid is 16 bytes (int, short, short, char[8])
    // so is a uuid_t.
    // I would be happier if this generate function was in QUuid.
    QUuid uuid;
    uuid_generate((unsigned char *)&uuid);
    return uuid;
#endif
}

/*!
  Returns the string stored for the custom field \a key.
  Returns a null string if the field does not exist.
 */
QString PimRecord::customField(const QString &key) const
{
    if (customMap.contains(key))
	return customMap[key];

    return QString::null;
}

/*!
  Sets the string stored for the custom field \a key to \a value.
 */
void PimRecord::setCustomField(const QString &key, const QString &value)
{
    if (customMap.contains(key))
	customMap.replace(key, value);
    else
	customMap.insert(key, value);

}

/*!
  Removes the custom field \a key.
 */
void PimRecord::removeCustomField(const QString &key)
{
    customMap.remove(key);
}

#ifndef QT_NO_DATASTREAM


QDataStream &operator<<(QDataStream &s, const QUuid& df)
{
    s << df.data1;
    s << df.data2;
    s << df.data3;
    for (int i = 0; i < 8; i ++)
	s << df.data4[i];
    return s;
}

QDataStream &operator>>(QDataStream &s, QUuid&df)
{
    s >> df.data1;
    s >> df.data2;
    s >> df.data3;
    for (int i = 0; i < 8; i ++)
	s >> df.data4[i];

    return s;
}

QDataStream &operator<<( QDataStream &s, const PimRecord &r )
{
    //<< all parts into s;
    s << r.mUid;
    s << r.mCategories.size();
    for (uint i = 0; i < r.mCategories.size(); i++) {
	s << r.mCategories[i];
    }
    s << r.customMap;
    return s;
}

QDataStream &operator>>( QDataStream &s, PimRecord &r )
{
    s >> r.mUid;
    uint size;
    s >> size;
    r.mCategories.resize(size);
    for (uint i = 0; i < size; i++) {
	s >> r.mCategories[i];
    }
    s >> r.customMap;
    return s;
}

#endif
