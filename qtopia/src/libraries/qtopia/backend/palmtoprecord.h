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
#ifndef QTPALMTOP_RECORD_H
#define QTPALMTOP_RECORD_H

#include <qtopia/qpeglobal.h>
#include "palmtopuidgen.h"
#include <qarray.h>
#include <qmap.h>

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

    void setCategories( const QArray<int> &v ) { mCats = v.copy(); mCats.sort(); }
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

