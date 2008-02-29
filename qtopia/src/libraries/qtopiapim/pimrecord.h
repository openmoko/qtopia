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

#ifndef PIM_RECORD_H
#define PIM_RECORD_H

#include <qtopia/pim/qtopiapim.h>
#include <qcstring.h>
#include <qmap.h>
#include <qvaluelist.h>
#include <qtopia/quuid.h>

class PimRecordPrivate;


struct QtopiaPimMapEntry {
    const char *identifier; // name
    const char *trName;
    int id;
    int uniqueness;
};

// field keys after this are reserved for custom fields.
#define CustomFieldsStart 0x40000000

class QTOPIAPIM_EXPORT PimRecord
{
public:
    enum PrivateFields {
	UID_ID = 0,
	PrivateFieldsEnd = 10
    };
    enum CommonFields {
	Categories = PrivateFieldsEnd,
	CommonFieldsEnd = 20
    };
    // The enum maps should internally UID and Category as keys, but the internal "private" enums should
    // be set using the above values as the keys. The #define below allows the enum declerations in
    // subclass to know which ID number is starting for their use


    PimRecord();
    PimRecord( const PimRecord &c );
    virtual ~PimRecord();

    virtual PimRecord &operator=( const PimRecord &c );

    bool operator==( const PimRecord &r ) const;
    bool operator!= ( const PimRecord &r ) const;

    void setCategories( const QArray<int> &categories );
    void setCategories( int id );
    void reassignCategoryId( int oldId, int newId );
    bool pruneDeadCategories(const QArray<int> &validCats);
    QArray<int> categories() const;

    virtual QString customField(const QString &) const;
    virtual void setCustomField(const QString &, const QString &);
    virtual void removeCustomField(const QString &);

    QUuid uid() const { return p_uid(); }
#ifdef QTOPIA_DESKTOP
    void setUid( QUuid u ) { p_setUid( u ); }
#endif

    virtual void setField(int,const QString &);
    virtual QString field(int) const;

    virtual void setFields(const QMap<int,QString> &);
    virtual QMap<int,QString> fields() const;

#ifndef QT_NO_DATASTREAM
    friend QTOPIAPIM_EXPORT QDataStream &operator>>( QDataStream &, PimRecord & );
    friend QTOPIAPIM_EXPORT QDataStream &operator<<( QDataStream &, const PimRecord & );
#endif

protected:

    static void initMaps(const char* trclass, const QtopiaPimMapEntry *, QMap<int,int> &, QMap<QCString,int> &,
			QMap<int,QCString> &, QMap<int,QString> &);
			
    // While it can store a full UID, we won't actually in the current
    // iteration.
    QUuid p_uid() const { return mUid; }
    void p_setUid( QUuid uid ) { mUid = uid; }

    QUuid mUid;
    QArray<int> mCategories;

    QMap<QString, QString> customMap;

private:
    PimRecordPrivate *d;
};


#ifndef QT_NO_DATASTREAM
#ifndef QTOPIA_DESKTOP
QTOPIAPIM_EXPORT QDataStream &operator<<(QDataStream &s, const QUuid& df);
QTOPIAPIM_EXPORT QDataStream &operator>>(QDataStream &s, QUuid&df);
#endif
QTOPIAPIM_EXPORT QDataStream &operator<<( QDataStream &, const PimRecord & );
QTOPIAPIM_EXPORT QDataStream &operator>>( QDataStream &, PimRecord & );
#endif

#endif
