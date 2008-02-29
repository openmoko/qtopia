/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNUGeneral Public License version 2 as published by the Free Software
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

    void setCategories( const QArray<int> &categories )
    {
	mCategories = categories;
    }
    void setCategories( int id )
    {
	mCategories.resize(1);
	mCategories[0] = id;
    }

    void reassignCategoryId( int oldId, int newId )
    {
	int index = mCategories.find( oldId );
	if ( index >= 0 )
	    mCategories[index] = newId;
    }

    bool pruneDeadCategories(const QArray<int> &validCats)
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
    
    QArray<int> categories() const { return mCategories; }

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
QTOPIAPIM_EXPORT QDataStream &operator<<(QDataStream &s, const QUuid& df);
QTOPIAPIM_EXPORT QDataStream &operator>>(QDataStream &s, QUuid&df);
QTOPIAPIM_EXPORT QDataStream &operator<<( QDataStream &, const PimRecord & );
QTOPIAPIM_EXPORT QDataStream &operator>>( QDataStream &, PimRecord & );
#endif

#endif
