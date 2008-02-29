/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#ifndef __QMAILFOLDERKEY_H
#define __QMAILFOLDERKEY_H

#include <QList>
#include <QVariant>
#include <qtopiaglobal.h>
#include <QSharedData>
#include "qmailstore.h"
#include "qmailfolder.h"

class QMailFolderKeyPrivate;

class QTOPIAMAIL_EXPORT QMailFolderKey
{
public:
    enum Operand
    {
        LessThan,
        LessThanEqual,
        GreaterThan,
        GreaterThanEqual,
        Equal,
        NotEqual,
        Contains
     };

	enum Property
    {
        Id,
        Name,
        ParentId
    };

public:
    QMailFolderKey(const Property& p, const QVariant& value, const Operand& = Equal);
    QMailFolderKey(const QMailFolderKey& other);
    virtual ~QMailFolderKey();

    QMailFolderKey operator~() const;
    QMailFolderKey operator&(const QMailFolderKey& other) const;
    QMailFolderKey operator|(const QMailFolderKey& other) const;
    QMailFolderKey& operator&=(const QMailFolderKey& other);
    QMailFolderKey& operator|=(const QMailFolderKey& other);

    bool operator==(const QMailFolderKey& other) const;
    bool operator !=(const QMailFolderKey& other) const;

    QMailFolderKey& operator=(const QMailFolderKey& other);

private:
    QMailFolderKey();
    void init(const Property& p, const Operand& op, const QVariant& value);

private:
	friend class QMailStore;
	friend class QMailStorePrivate;

private:
    QSharedDataPointer<QMailFolderKeyPrivate> d;

};

#endif
