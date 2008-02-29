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
#ifndef QTPALMTOP_RECORD_H
#define QTPALMTOP_RECORD_H
#include <qtopia/qpeglobal.h>
#include "palmtopuidgen.h"
#include <qarray.h>
#include <qmap.h>

#if defined(QTOPIA_TEMPLATEDLL)
// MOC_SKIP_BEGIN
QTOPIA_TEMPLATEEXTERN template class QTOPIA_EXPORT QMap<QString, QString>;
// MOC_SKIP_END
#endif

class QRegExp;
namespace Qtopia {

class RecordPrivate;
class QTOPIA_EXPORT Record
{
public:
    Record() : mUid(0), mCats() { }
    Record( const Record &c ) :	mUid( c.mUid ), mCats ( c.mCats ), customMap(c.customMap) { }
    virtual ~Record() { }

    Record &operator=( const Record &c );

    virtual bool match( const QRegExp & ) const { return FALSE; }

    void setCategories( const QArray<int> &v ) { mCats = v; mCats.sort(); }
    void setCategories( int single );
    const QArray<int> &categories() const { return mCats; }

    void reassignCategoryId( int oldId, int newId )
    {
	int index = mCats.find( oldId );
	if ( index >= 0 )
	    mCats[index] = newId;
    }

    int uid() const { return mUid; };
    virtual void setUid( int i ) { mUid = i; uidGen().store( mUid ); }
    bool isValidUid() const { return mUid != 0; }
    void assignUid() { setUid( uidGen().generate() ); }

    virtual QString customField(const QString &) const;
    virtual void setCustomField(const QString &, const QString &);
    virtual void removeCustomField(const QString &);

    virtual bool operator == ( const Record &r ) const
{ return mUid == r.mUid; }
    virtual bool operator != ( const Record &r ) const
{ return mUid != r.mUid; }

    // convenience methods provided for loading and saving to xml
    static QString idsToString( const QArray<int> &ids );
    // convenience methods provided for loading and saving to xml
    static QArray<int> idsFromString( const QString &str );

    // for debugging
    static void dump( const QMap<int, QString> &map );

protected:
    virtual UidGen &uidGen() = 0;
    virtual QString customToXml() const;
private:
    int mUid;
    QArray<int> mCats;
    QMap<QString, QString> customMap;
    RecordPrivate *d;
};

}

#endif

