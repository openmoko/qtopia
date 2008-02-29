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
// the next few lines have to come first
#include <qtopia/qpeglobal.h>
#ifdef Q_OS_WIN32
#include <objbase.h>
#else
extern "C" {
#include <uuid/uuid.h>
#define UUID_H_INCLUDED
}
#endif

#include "record.h"

#include <qtopia/quuid.h>
#include <qtopia/timeconversion.h>

//using namespace qpe;


/*?
  \class Record
  \internal
  \brief The Record class is a record in the Database.
  
  The Record class represents one record stored (or to be stored in
  the Qt Palmtop Database. The class is optimised for fast reading and
  writing. A record can consist of a series of fields, represented by
  integer keys. Each field is stored with a type information, and
  typechecking is done when reading the fields.
*/

/*?
  \enum Record::FieldType
  \value UndefinedType Indicated the field does not exist in this record
  \value BoolType This field contains a boolean
  \value IntType the field contains an integer
  \value DateType the field contains a Date
  \value DateTimeType the field contains a datetime
  \value StringType the field contains a string
  \endenum
  
  The type of a field can be queried with the typeof() method.

*/


/*?
  will create an empty record.
*/
Record::Record()
{
    mIndex = -1;
    initEmpty();
}

Record::~Record()
{
}

void Record::initEmpty()
{
    data.resize( sizeof( RecordHeader ) );
    memset( data.data(), 0, data.size() );
    uid();
}

QUuid Record::generateUuid()
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
#if QT_VERSION < 0x030000
    uuid_t uuid;
    uuid_generate( uuid );

    return QUuid( uuid );
#else
    return QUuid();
#endif
#endif
}

/*?
  returns the type of the fiels \a fieldId.
*/
Record::FieldType Record::typeOf( int fieldId )
{
    AttributeEntry *attr = findAttribute( fieldId );
    if ( !attr )
	return UndefinedType;
    return (FieldType)attr->type();
}


Record::AttributeEntry *Record::findAttribute( int id ) const
{
    RecordHeader *h = header();
    if ( h ) {
	AttributeEntry *attr = firstAttribute();
	//qDebug("looking for attr %d total=%d", id, h->numAttributes() );
	for ( int i = 0; i < h->numAttributes(); i ++ ) {
	    //qDebug("attribute = [ %d, %d, %x]", attr->id(), attr->type(), attr->offsetOrData() );
	    if ( attr->id() == id ) {
		//qDebug("found" );
		return attr;
	    }
	    attr++;
	}
    }
    return 0;
}

Record::AttributeEntry *Record::findAttribute( int id, bool add )
{
    RecordHeader *h = header();
    AttributeEntry *attr = firstAttribute();
    for ( int i = 0; i < h->numAttributes(); i ++ ) {
	if ( attr->id() == id )
	    return attr;
	attr++;
    }
    if ( add ) {
	// add the new attribute
	// first make space for the attribute
	int insertPoint = dataOffset();
	insertBytes( insertPoint, sizeof( AttributeEntry ) );

	AttributeEntry *attr = (AttributeEntry *)( data.data() + insertPoint );
	attr->setId( id );
	attr->setType( UndefinedType );
	attr->setOffsetOrData( 0 );
	header()->setNumAttributes( header()->numAttributes() + 1 );
	//qDebug("added attribute, now have %d", header()->numAttributes() );
	return attr;
    }
    return 0;
}

/*?
  Sets the set of categories to which the Record belongs to \a v.
*/
void Record::setCategories( const QArray<int> &v )
{
    uint num = v.count();
    RecordHeader *h = header();
    if ( h->numCategories() > num ) {
	removeBytes( categoryOffset(), sizeof(int) * (h->numCategories() - num ) );
    } else if ( h->numCategories() < num ) {
	qDebug("insertBytes in categories");
	insertBytes( categoryOffset(), sizeof(int) * (num - h->numCategories() ) );
    }
    int *ncat = (int *) ( data.data() + categoryOffset() );
    for ( int i = 0;  i < num; i++ ) 
	ncat[i] = htonl( v[i] );
    catsDirty = TRUE;
}

/*?
  Sets the record to belong only to the category with the id \a single.
*/
void Record::setCategories( int single )
{
    QArray<int> cats;
    cats.resize( 1 );
    cats[0] = single;
    setCategories( cats );
    catsDirty = TRUE;
}

/*?
  returns the list of categories the record belongs to
*/
QArray<int> Record::categories() const
{
    RecordHeader *h = header();
    uint num = h->numCategories();
    QArray<int> cats;
    cats.resize( num );
    int *ncat = (int *) ( data.data() + categoryOffset() );
    for ( int i = 0; i < num; i++ ) 
	cats[i] = ntohl( ncat[i] );
    return cats;
}

/*?
  Returns the unique Id for this record. In case it doesn't exist, a new unique Id
  will be created.
*/
QUuid Record::uid() const
{
    return header()->uid();
}

/*?
  Compare this record with \a other. The default implementation
  compares the values of the field index 0.
*/
int Record::compare( const Record &other ) const
{
    AttributeEntry *thisAttr = findAttribute( 0 );
    AttributeEntry *otherAttr = other.findAttribute( 0 );
    if ( !thisAttr && !otherAttr )
	return 0;
    if ( !thisAttr )
	return 1;
    if ( !otherAttr )
	return -1;
    if ( thisAttr->type() != otherAttr->type() )
	return 0;
    switch( thisAttr->type() ) {
	case UndefinedType:
	    return 0;
	case BoolType:
	    return ( thisAttr->offsetOrData() == 0 ) - ( otherAttr->offsetOrData() == 0 );
	case IntType:
	case DateType:
	case DateTimeType:
	    return ( thisAttr->offsetOrData() - otherAttr->offsetOrData() );
	case StringType: {
	    QString thisstr = stringField( 0 );
	    QString otherstr = other.stringField( 0 );
	    return thisstr.compare( otherstr );
	}
    }
    return 0;
}


/*?
  Returns the date stored in the field \a id. If the field is not of type Date, *\a ok
  will be FALSE.  Returns an invalid date if the field does not exist.
*/
QDate Record::dateField( int id, bool *ok ) const
{
    AttributeEntry *attr = findAttribute( id );
    if ( attr && attr->type() == DateType ) {
	if ( ok ) *ok = TRUE;
	uint date = attr->offsetOrData();
	return TimeConversion::fromUTC( attr->offsetOrData() ).date();
    }
    if ( ok ) *ok = !attr; // ok is true if we didn't find an entry
    return QDate();
}

/*?
  Returns the datetime stored in the field \a id. If the field is not of type DateTime,
  *\a ok will be FALSE.  Returns an invalid datetime if the field does not exist.
*/
QDateTime Record::dateTimeField( int id, bool *ok ) const
{
    AttributeEntry *attr = findAttribute( id );
    if ( attr && attr->type() == DateTimeType ) {
	if ( ok ) *ok = TRUE;
	return TimeConversion::fromUTC( attr->offsetOrData() );
    }
    if ( ok ) *ok = !attr; // ok is true if we didn't find an entry
    return QDateTime();
}

/*?
  Returns the integer stored in the field \a id. If the field is not of type Int,
  *\a ok will be FALSE.  Returns 0 if the field does not exist.
*/
int Record::intField( int id, bool *ok ) const
{
    AttributeEntry *attr = findAttribute( id );
    if ( attr && attr->type() == IntType ) {
	if ( ok ) *ok = TRUE;
	return attr->offsetOrData();
    }
    if ( ok ) *ok = !attr; // ok is true if we didn't find an entry
    return 0;
}

/*?
  Returns the string stored in the field \a id. If the field is not of type String,
  *\a ok will be false.  Returns a null string if the field does not exist.
*/
QString Record::stringField( int id, bool *ok ) const
{
    AttributeEntry *attr = findAttribute( id );
    //qDebug("looking for string, attr=%d, field found = [%d, %d, %d]", id, attr->id(), attr->type(), attr->offsetOrData() );
    if ( attr && attr->type() == StringType ) {
	if ( ok ) *ok = TRUE;
	int off = dataOffset() + attr->offsetOrData();
	//qDebug("string at offset %x", off );
	//qDebug("data at offset = %x", 
	//       *((int *)(data.data()+off))
	//    );
	int bytesRemaining = data.size() - off;
	if ( bytesRemaining > 4 ) {
	    uint len = ntohl( *((int *) (data.data() + off )) );
	    if ( len*sizeof(QChar) <= (uint)bytesRemaining - 4 ) {
		QString str;
		str.setUnicode( (QChar *)(data.data() + off + 4), len );
		//qDebug("string is %s, len should be %d", str.latin1(), len );
		// ### take care of byte ordering
		return str;
	    }
	}
    }
    if ( ok ) *ok = !attr; // ok is true if we didn't find an entry
    return QString::null;  
}

/*?
  Returns the boolean stored in the field \a id. If the field is not of type Bool,
  *\a ok, will be FALSE.  Returns TRUE if the field does not exist.
*/
bool Record::boolField( int id, bool *ok ) const
{
    AttributeEntry *attr = findAttribute( id );
    if ( attr && attr->type() == BoolType ) {
	if ( ok ) *ok = TRUE;
	return (bool) attr->offsetOrData();
    }
    if ( ok ) *ok = !attr; // ok is true if we didn't find an entry
    return TRUE;
}

/*?
  Sets the field \a attrId to \a date. The type is set to Date by this call.
*/
void Record::setDateField( int attrId, const QDate &date )
{
    AttributeEntry *attr = findAttribute( attrId, TRUE );
    attr->setType( DateType );
    attr->setOffsetOrData( TimeConversion::toUTC( date ) );
}

/*?
  Sets the field \a attrId to date and time \a dt. The type is set to DateTime by this call.
*/
void Record::setDateTimeField( int attrId, const QDateTime &dt ) 
{
    AttributeEntry *attr = findAttribute( attrId, TRUE );
    attr->setType( DateTimeType );
    attr->setOffsetOrData( TimeConversion::toUTC( dt ) );    
}

/*?
  Sets the field \a attrId to the integer \a i. The type is set to Integer by this call.
*/
void Record::setIntField( int attrId, int i )
{
    AttributeEntry *attr = findAttribute( attrId, TRUE );
    attr->setType( IntType );
    attr->setOffsetOrData( i );
}

/*?
  Sets the field \a attrId to the string \a str. The type is set to String by this call.
*/
void Record::setStringField( int attrId, const QString &str )
{
    AttributeEntry *attr = findAttribute( attrId, TRUE );
    if ( attr->type() == StringType ) {
	// delete old string
	int off = dataOffset() + attr->offsetOrData();
	int len = ntohl( *((int *)(data.data() + off ) ) )*sizeof(QChar) + 4;
	//qDebug("deleting old string, removing %d bytes at %d", len, off );
	removeBytes( off, len );
    }
    attr->setType( StringType );
    // add new string
    int len = str.length()*sizeof( QChar ) + 4;
    int point = data.size();
    data.resize( point + len );
    int *nstrlen = (int *)(data.data() + point);
    *nstrlen = htonl( str.length() );
    char *d = (data.data() + point + 4);
    memcpy( d, str.unicode(), str.length() *sizeof( QChar ) );

    attr->setOffsetOrData( point - dataOffset() );
}

/*?
  Sets the field \a attrId to the boolean \a b. The type is set to Bool by this call.
*/
void Record::setBoolField( int attrId, bool b )
{
    AttributeEntry *attr = findAttribute( attrId, TRUE );
    attr->setType( BoolType );
    attr->setOffsetOrData( (int)b );    
}


void Record::insertBytes( int point, uint bytes )
{
    if ( bytes == 0 )
	return;
    int size = data.size() - point;
    data.resize( data.size() + bytes );
    memmove( data.data() + point + bytes, data.data() + point, size );
#ifdef RECORD_DEBUG
    memset( data.data() + point, 0xde, bytes );
#endif    
}

void Record::removeBytes( int point, uint bytes )
{
    if ( bytes == 0 )
	return;
    int size = data.size() - point - bytes;
    memmove( data.data() + bytes, data.data() + point + bytes, size );        
    data.resize( data.size() - bytes );
}
