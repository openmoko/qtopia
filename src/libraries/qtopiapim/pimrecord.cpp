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
#include <qtopia/pim/private/xmlio_p.h>
#include <qdatastream.h>
#include <qtranslator.h>
#include <qtopia/qpeapplication.h>
#include <qobject.h>
#include <stdlib.h>

#if defined(Q_WS_WIN32)
#define INITGUID
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
  \fn void PimRecord::reassignCategoryId( int, int )
  \internal
*/

/*!
  \fn bool PimRecord::pruneDeadCategories( const QArray<int> & )
  \internal

  Removes deleted categories from the record.  Returns
  TRUE if any categories were removed, FALSE otherwise.
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
  \enum PimRecord::PrivateFields
  \internal
*/

/*!
  \enum PimRecord::CommonFields
  \internal
*/

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

/*!
  \internal
*/
void PimRecord::setField(int key,const QString &s)
{
    if ( key == Categories ) {
	setCategories( PimXmlIO::idsFromString( s ) );
    } else if ( key == UID_ID ) {
	// DO NOT USE THIS WHEN CREATING A NEW CLASS
	// this should only get called by the fromMap methods of the subclasses
	QUuid id;
	id.data1 = s.toInt();
	p_setUid( id );
    } else
	qWarning("PimRecord::setField() Did not get passed a valid key %d", key);
}

/*!
  \internal
*/
QString PimRecord::field(int key) const
{
    if ( key == Categories ) {
	return PimXmlIO::idsToString( mCategories );
    } else if ( key == UID_ID ) {
	return QString::number( PimXmlIO::uuidToInt( mUid ) );
    }

    qWarning("PimRecord::field() Did not get passed a valid key %d", key);

    return QString::null;
}

/*!
  \internal
*/
void PimRecord::setFields(const QMap<int,QString> &m)
{
    for (QMap<int,QString>::ConstIterator it = m.begin(); it!= m.end(); ++it)
    	setField( it.key(), (*it) );
}

/*!
  \internal
*/
QMap<int,QString> PimRecord::fields() const
{
    QMap<int,QString> m;

    QString str  = field(UID_ID);

    if ( !str.isEmpty() )		//TODO: check whether we need to use QString::null instead
	m.insert(UID_ID, str );
    str = field(Categories);
    if ( !str.isEmpty() )		//TODO: check whether we need to use QString::null instead
	m.insert(Categories, str );

    return m;
}

static const QtopiaPimMapEntry recentries[] = {
    { "Uid", NULL, PimRecord::UID_ID, 0 },
    { "Categories", // No tr
	    QT_TRANSLATE_NOOP("PimRecord", "Categories"), PimRecord::Categories, 0 },

    { 0, 0, 0, 0 }
};

/*!
  \internal
*/
void PimRecord::initMaps(const char* trclass, const QtopiaPimMapEntry *entries, QMap<int,int> &uniquenessMap, QMap<QCString,int> &identifierToKeyMap,
			QMap<int,QCString> &keyToIdentifierMap, QMap<int,QString> &trFieldsMap)
{
    static int translation_installed = 0;
    if ( !translation_installed ) {
	++translation_installed;
	QString lang = getenv("LANG");
	QTranslator * trans = new QTranslator(qApp);
	QString tfn = QPEApplication::qpeDir()+"i18n/"+lang+"/libqpepim.qm";
qDebug("Load %s",tfn.latin1());
	if ( trans->load( tfn ))
	    qApp->installTranslator( trans );
	else
	    delete trans;
    }

    for ( const QtopiaPimMapEntry *entry = recentries; entry->identifier; ++entry ) {
   	keyToIdentifierMap.insert( entry->id, entry->identifier );
	identifierToKeyMap.insert( entry->identifier, entry->id );
	if ( entry->trName )
	    trFieldsMap.insert( entry->id, qApp->translate("PimRecord", entry->trName) );
	uniquenessMap.insert( entry->id, entry->uniqueness );
    }

    for ( const QtopiaPimMapEntry *entry2 = entries; entry2->identifier; ++entry2 ) {
	keyToIdentifierMap.insert( entry2->id, entry2->identifier );
	identifierToKeyMap.insert( entry2->identifier, entry2->id );
	if ( entry2->trName )
	    trFieldsMap.insert( entry2->id, qApp->translate(trclass, entry2->trName) );
	uniquenessMap.insert( entry2->id, entry2->uniqueness );
    }
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
    for (int i = 0; i < (int)r.mCategories.size(); i++) {
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
    for (int i = 0; i < (int)size; i++) {
	s >> r.mCategories[i];
    }
    s >> r.customMap;
    return s;
}

#endif
