/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef PIM_RECORD_H
#define PIM_RECORD_H

#include <quniqueid.h>
#include <qmap.h>
#include <qlist.h>
#include <quuid.h>

class QPimRecordPrivate;

class QTOPIAPIM_EXPORT QPimRecord
{
public:
    virtual ~QPimRecord();
    // The enum maps should internally UID and Category as keys, but the internal "private" enums should
    // be set using the above values as the keys. The #define below allows the enum declerations in
    // subclass to know which ID number is starting for their use

    bool operator==( const QPimRecord &other ) const;
    bool operator!=( const QPimRecord &other ) const;

    void setCategories( const QList<QString> &categories );
    void setCategories( const QString &id );
    void reassignCategoryId( const QString & oldId, const QString & newId );
    bool pruneDeadCategories(const QList<QString> &validCats);
    QList<QString> categories() const;

    virtual QString customField(const QString &) const;
    virtual void setCustomField(const QString &, const QString &);
    virtual void removeCustomField(const QString &);
    QMap<QString, QString> customFields() const { return customFieldsRef(); }
    void setCustomFields(const QMap<QString, QString> &fields) { customFieldsRef() = fields; }

    QUniqueId uid() const { return uidRef(); }
    void setUid( const QUniqueId & uid ) { uidRef() = uid; }

    virtual void setNotes(const QString &) = 0;
    virtual QString notes() const = 0;

protected:
    // expect overridden classes?  const and non-const refes?
    virtual QUniqueId &uidRef() = 0;
    virtual const QUniqueId &uidRef() const = 0;

    virtual QList<QString> &categoriesRef() = 0;
    virtual const QList<QString> &categoriesRef() const = 0;

    virtual QMap<QString, QString> &customFieldsRef() = 0;
    virtual const QMap<QString, QString> &customFieldsRef() const = 0;
};

#endif
