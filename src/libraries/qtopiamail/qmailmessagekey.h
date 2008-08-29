/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef __QMAILMESSAGEKEY_H
#define __QMAILMESSAGEKEY_H

#include <QList>
#include <QVariant>
#include <QSharedData>
#include <QFlags>
#include <qtopiaglobal.h>
#include "qmailmessage.h"

class QMailMessageKeyPrivate;

class QTOPIAMAIL_EXPORT QMailMessageKey
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
        Id = 0x0001,
        Type = 0x0002,
        ParentFolderId = 0x0004,
        Sender = 0x0008,
        Recipients = 0x0010,
        Subject = 0x0020,
        TimeStamp = 0x0040,
        Status = 0x0080,
        FromAccount = 0x0100,
        FromMailbox = 0x0200,
        ServerUid = 0x0400,
        Size = 0x0800,
    };
    Q_DECLARE_FLAGS(Properties,Property)

public:
    QMailMessageKey();
    QMailMessageKey(const Property& p, const QVariant& value, const Operand& c = Equal);
    explicit QMailMessageKey(const QMailIdList& ids);
    QMailMessageKey(const QMailMessageKey& other);
    virtual ~QMailMessageKey();

    QMailMessageKey operator~() const;
    QMailMessageKey operator&(const QMailMessageKey& other) const;
    QMailMessageKey operator|(const QMailMessageKey& other) const;
    QMailMessageKey& operator&=(const QMailMessageKey& other);
    QMailMessageKey& operator|=(const QMailMessageKey& other);

    bool operator==(const QMailMessageKey& other) const;
    bool operator !=(const QMailMessageKey& other) const;

    QMailMessageKey& operator=(const QMailMessageKey& other);

    bool isEmpty() const;

private:
    friend class QMailStore;
    friend class QMailStorePrivate;

private:
    QSharedDataPointer<QMailMessageKeyPrivate> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QMailMessageKey::Properties)

#endif
