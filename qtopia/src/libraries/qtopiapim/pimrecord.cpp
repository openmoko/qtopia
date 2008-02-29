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
#define QTOPIA_INTERNAL_LOADTRANSLATIONS
#include "pimrecord.h"
#include <qtopia/pim/private/xmlio_p.h>
#include <qtopia/private/vobject_p.h>
#include <qtopia/categories.h>
#include <qdatastream.h>
#include <qtextcodec.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>
#include <qobject.h>
#include <stdlib.h>

static QMap<QString, int> *cidToKeyMapPtr = 0;
static QMap<int, QString> *keyToCidMapPtr = 0;
static int nextCidKey = CustomFieldsStart;

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
	: mUid(other.mUid), mCategories(other.mCategories.copy()),
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
    mCategories = other.mCategories.copy();
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

void PimRecord::setCategories( const QArray<int> &categories )
{
    mCategories = categories.copy();
}

void PimRecord::setCategories( int id )
{
    mCategories.resize(1);
    mCategories[0] = id;
}

void PimRecord::reassignCategoryId( int oldId, int newId )
{
    // workaround for qt bug which gives qWarnings on calling find on an empty array
    if ( !mCategories.count() )
	return;

    int index = mCategories.find( oldId );
    if ( index >= 0 )
	mCategories[index] = newId;
}

bool PimRecord::pruneDeadCategories(const QArray<int> &validCats)
{
    QArray<int> newCats;

    for (int i = 0; i < (int) mCategories.count(); i++ ) {
	if ( validCats.contains( mCategories[i] ) ) {
	    newCats.resize( newCats.count() + 1 );
	    newCats[(int)newCats.count() - 1] = mCategories[i];
	}
    }

    if ( newCats.count() != mCategories.count() ) {
	mCategories = newCats;
	return TRUE;
    }

    return FALSE;
}

QArray<int> PimRecord::categories() const
{
    return mCategories;
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
    if (!cidToKeyMapPtr->contains(key)) {
	// need a new int for this key.A
	cidToKeyMapPtr->insert(key, nextCidKey);
	keyToCidMapPtr->insert(nextCidKey, key);
	nextCidKey++;
    }
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
    } else if ( keyToCidMapPtr->contains(key) ) {
	setCustomField((*keyToCidMapPtr)[key], s);
    } else {
	qWarning("PimRecord::setField() Did not get passed a valid key %d", key);
    }
}

/*!
  \internal
*/
QString PimRecord::field(int key) const
{
    if ( key == Categories ) {
	return PimXmlIO::idsToString( mCategories );
    } else if ( key == UID_ID ) {
	return mUid.isNull() ? QString::null :
	    QString::number( PimXmlIO::uuidToInt( mUid ) );
    } else if ( keyToCidMapPtr->contains(key) ) {
	return customField((*keyToCidMapPtr)[key]);
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

    // now need to do the keys for custom fields.
    QMap<int,QString>::Iterator it;
    for( it = keyToCidMapPtr->begin(); it != keyToCidMapPtr->end(); ++it ) {
	if ( customMap.contains(it.data()) )
	    m.insert(it.key(), customMap[ it.data() ] );
    }

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
    // init private custom->key maps.
    cidToKeyMapPtr = new QMap<QString, int>;
    keyToCidMapPtr = new QMap<int, QString>;

#ifndef QT_NO_TRANSLATION
    static int translation_installed = 0;
    if ( !translation_installed ) {
	++translation_installed;
	QPEApplication::loadTranslations("libqpepim");
    }
#endif

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

static QTextCodec* vobj_codec=0;
static Categories* cats=0;
static QStringList* comps=0;
static int catschanged=0;
static void startCats()
{
    cats = new Categories;
    cats->load(categoryFileName());
    catschanged = 0;
}
static void endCats()
{
    if ( catschanged )
	cats->save(categoryFileName());
    delete cats;
    cats = 0;
}

void qpe_startVObjectInput()
{
    startCats();
}

void qpe_startVObjectOutput()
{
    Config pimConfig( "Beam" );
    pimConfig.setGroup("Send");
    QString cs = "UTF-8";
    QString dc = pimConfig.readEntry("DeviceConfig");
    if ( !dc.isEmpty() ) {
	Config devcfg(pimConfig.readEntry("DeviceConfig"),Config::File);
	if ( devcfg.isValid() ) {
	    devcfg.setGroup("Send");
	    cs = devcfg.readEntry("CharSet","UTF-8");
	    QString comp = devcfg.readEntry("Compatibility");
	    comps = new QStringList(QStringList::split(' ',comp));
	}
    }
    vobj_codec = QTextCodec::codecForName(cs.latin1());
    startCats();
}

bool qpe_vobjectCompatibility(const char* misfeature)
{
    return comps && comps->contains(misfeature);
}

void qpe_setVObjectProperty(const QString& name, const QString& value, const char* type, PimRecord* r)
{
    if ( name == VCCategoriesProp ) {
	QStringList cl = QStringList::split(';',value);
	QArray<int> ca(cl.count());
	int nid=0;
	for (QStringList::ConstIterator it=cl.begin(); it!=cl.end(); ++it) {
	    QString cname = *it;
	    if ( cname.left(2) == "X-" )
		cname = cname.mid(2);
	    int id = cats->id(type,cname);
	    if ( !id ) {
		QString cnamel = cname.lower();
		if ( cnamel != cname ) {
		    // Try case-insensitive for global categories
		    QStringList gc = cats->globalCategories();
		    for (QStringList::ConstIterator git=gc.begin(); git!=gc.end(); ++git) {
			if ( (*git).lower() == cnamel || (*git).lower() == "_"+cnamel ) { // check with _ for global categories
			    cname = *git;
			    id = cats->id(type,cname);
			    break;
			}
		    }
		}
	    }
	    if ( !id && cname != "Unfiled" ) { // No tr
		if ( !cats->exists(cname) ) { // Same category, different app - ignore
		    id = cats->addCategory(type,cname);
		    catschanged++;
		}
	    }
	    if ( id )
		ca[nid++] = id;
	}
	ca.resize(nid);
	r->setCategories(ca);
    }
}

VObject *qpe_safeAddPropValue( VObject *o, const char *prop, const QString &value )
{
    VObject *ret = 0;
    if ( o && !value.isEmpty() ) {
	if ( vobj_codec )
	    ret = addPropValue( o, prop, vobj_codec->fromUnicode(value) );
	else
	    ret = addPropValue( o, prop, value.latin1() ); // NOT UTF-8, that is by codec
    }
    return ret;
}

VObject *qpe_safeAddProp( VObject *o, const char *prop)
{
    VObject *ret = 0;
    if ( o )
	ret = addProp( o, prop );
    return ret;
}

void qpe_endVObjectInput()
{
    endCats();
}

void qpe_endVObjectOutput(VObject *o, const char* type, const PimRecord* r)
{
    QStringList c = cats->labels(type,r->categories());
    for( uint i = 0 ; i < c.count() ; ++i )
    {
	QString cur = c[i];
	if( cur.startsWith("_") ) // global category
	    c[i] = cur.right( cur.length()-1 );

    }
    qpe_safeAddPropValue( o, VCCategoriesProp, c.join(";") );
    delete comps;
    comps = 0;
    endCats();
}

#ifndef QT_NO_DATASTREAM

#ifndef QTOPIA_DESKTOP
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
#endif

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
