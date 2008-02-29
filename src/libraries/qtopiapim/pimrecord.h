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

#ifndef PIM_RECORD_H
#define PIM_RECORD_H

#include <qtopia/qpeglobal.h>
#include <qcstring.h>
#include <qmap.h>
#include <qtopia/quuid.h>




class PimRecordPrivate;

class QTOPIA_EXPORT PimRecord
{
public:
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

    QArray<int> categories() const { return mCategories; } 

    virtual QString customField(const QString &) const;
    virtual void setCustomField(const QString &, const QString &);
    virtual void removeCustomField(const QString &);

    QUuid uid() const { return p_uid(); }
    //void setUid( QUuid u ) { p_setUid( u ); }

#ifndef QT_NO_DATASTREAM
    friend QTOPIA_EXPORT QDataStream &operator>>( QDataStream &, PimRecord & );
    friend QTOPIA_EXPORT QDataStream &operator<<( QDataStream &, const PimRecord & );
#endif

protected:

    // While it can store a full UID, we won't actually in the current
    // iteration.
    QUuid p_uid() const { return mUid; }
    void p_setUid( QUuid uid ) { mUid = uid; }

    QUuid mUid;
    QArray<int> mCategories;

    QMap<QString, QString> customMap;

private:
    static QUuid generateUuid();
    PimRecordPrivate *d;
};


#ifndef QT_NO_DATASTREAM
QTOPIA_EXPORT QDataStream &operator<<(QDataStream &s, const QUuid& df);
QTOPIA_EXPORT QDataStream &operator>>(QDataStream &s, QUuid&df);
QTOPIA_EXPORT QDataStream &operator<<( QDataStream &, const PimRecord & );
QTOPIA_EXPORT QDataStream &operator>>( QDataStream &, PimRecord & );
#endif

#endif
