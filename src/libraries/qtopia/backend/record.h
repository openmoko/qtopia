/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** Licensees holding valid Qtopia Developer license may use this
** file in accordance with the Qtopia Developer License Agreement
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE.
**
** email sales@trolltech.com for information about Qtopia License
** Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef RECORD_H
#define RECORD_H

#include <qtopia/qpeglobal.h>
#include <qtopia/quuid.h>

#include <qarray.h>
#include <qstring.h>
#include <qdatetime.h>

#include <sys/types.h>
#include <netinet/in.h>

class QRegExp;
//namespace qpe {

class RecordPrivate;
class QTOPIA_EXPORT Record
{
    friend class RecordPrivate;
    friend class Database;
    friend class DatabaseView;
public:
    enum FieldType {
	UndefinedType,
	BoolType,
	IntType,
	DateType,
	DateTimeType,
	StringType
    };

    Record( const QByteArray &ba ) : mIndex( -1 ), data( ba ), catsDirty( FALSE ) 
    { if ( !ba.data() ) initEmpty(); }
    Record();
    Record( const Record &c ) : mIndex( c.mIndex), data( c.data.copy() ), catsDirty( c.catsDirty ) { }
    virtual ~Record();
    
    Record &operator=( const Record &c )
    { mIndex = c.mIndex; data = c.data.copy(); catsDirty = c.catsDirty; return *this; }

    virtual bool match( QRegExp & ) { return FALSE; }
    virtual int compare( const Record &other ) const;

    void setCategories( const QArray<int> &v );
    void setCategories( int single );
    QArray<int> categories() const;

    QUuid uid() const;

    bool operator == ( const Record &r ) const { return header()->uid() == r.header()->uid(); }
    bool operator != ( const Record &r ) const { return header()->uid() != r.header()->uid(); }

    FieldType typeOf( int fieldId );
    
    QDate dateField( int fieldId, bool *ok = 0 ) const;
    QDateTime dateTimeField( int fieldId, bool *ok = 0 ) const;
    int intField( int fieldId, bool *ok = 0 ) const;
    QString stringField( int fieldId, bool *ok = 0 ) const;
    bool boolField( int fieldId, bool *ok = 0 ) const;

    void setDateField( int fieldId, const QDate &date );
    void setDateTimeField( int fieldId, const QDateTime & );
    void setIntField( int fieldId, int i );
    void setStringField( int fieldId, const QString &str );
    void setBoolField( int fieldId, bool b );
    
private:
    void initEmpty();
    

    struct RecordHeader {
    private:
	QUuid uuid;
	uint numCats;
	ushort numAttr;
    public:	
	QUuid uid() { 
	    if ( uuid.isNull() ) {
		uuid = Record::generateUuid(); 
	    }
	    return uuid; 
	}
	void setUid( QUuid uid ) { uuid = uid; }
	uint numCategories() { return ntohl( numCats ); }
	ushort numAttributes() { return ntohs( numAttr ); }
	void setNumCategories( uint num ) { numCats = htonl( num ); }
	void setNumAttributes( ushort num ) { numAttr = htons( num ); }
    };
    struct AttributeEntry {
    private:
	ushort nid;
	ushort ntype;
	int noffsetOrData;
    public:	
	ushort id() { return ntohs( nid ); }
	ushort type() { return ntohs( ntype ); }
	uint offsetOrData() { return ntohl( noffsetOrData ); }
	void setId( ushort num ) { nid = htons( num ); }
	void setType( ushort num ) { ntype = htons( num ); }
	void setOffsetOrData( uint num ) { noffsetOrData = htonl( num ); }
    };
    
	
    
    RecordHeader *header() const { return (RecordHeader *)data.data(); }
    AttributeEntry *firstAttribute() const {
	RecordHeader *h = header();
	return ( (h && h->numAttributes()) ? 
		 (AttributeEntry *) ( data.data() + sizeof( RecordHeader ) + 4*h->numCategories() ) :
		 0 );
    }
    int categoryOffset() const {
	return sizeof( RecordHeader );
    }	
    int attributeOffset() const {
	return sizeof( RecordHeader ) +
	    sizeof(int) * header()->numCategories();
    }
    int dataOffset() const {
	RecordHeader *h = header();
	return ( sizeof( RecordHeader ) + 
		 sizeof(int) * h->numCategories() + 
		 sizeof( AttributeEntry ) * h->numAttributes() );
    }
    AttributeEntry *findAttribute( int id ) const;
    AttributeEntry *findAttribute( int id, bool add  );
    
    void insertBytes( int point, uint bytes );
    void removeBytes( int point, uint bytes );
    
    friend struct RecordHeader;
    static QUuid generateUuid();

    int mIndex;
    QByteArray data;
    bool catsDirty : 1;
};

//};

#endif
