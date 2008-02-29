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

#ifndef __QMAILMESSAGEKEY_H
#define __QMAILMESSAGEKEY_H

#include <QList>
#include <QVariant>
#include <QSharedData>
#include <qtopiaglobal.h>
#include "qmailstore.h"
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
        Id,
        Type,
        ParentFolderId,
        Sender,
        Recipients,
        Subject,
        TimeStamp,
        Flags,
        FromAccount,
        FromMailbox,
        ServerUid,      
        Size
    };

public:
    QMailMessageKey();
    QMailMessageKey(const Property& p, const QVariant& value, const Operand& c = Equal);
    QMailMessageKey(const QMailMessageKey& other);
    virtual ~QMailMessageKey();

    bool isEmpty() const;

    QMailMessageKey operator~() const;
    QMailMessageKey operator&(const QMailMessageKey& other) const;
    QMailMessageKey operator|(const QMailMessageKey& other) const;
    QMailMessageKey& operator&=(const QMailMessageKey& other);
    QMailMessageKey& operator|=(const QMailMessageKey& other);

    bool operator==(const QMailMessageKey& other) const;
    bool operator !=(const QMailMessageKey& other) const;

    QMailMessageKey& operator=(const QMailMessageKey& other);

private:
    void init();
    void init(const Property& p, const Operand& op, const QVariant& value);

private:
	friend class QMailStore;
	friend class QMailStorePrivate;

private:
    QSharedDataPointer<QMailMessageKeyPrivate> d;
};

#endif
