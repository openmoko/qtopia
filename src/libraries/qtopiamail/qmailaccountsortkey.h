/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#ifndef QMAILACCOUNTSORTKEY_H
#define QMAILACCOUNTSORTKEY_H

#include <qtopiaglobal.h>
#include <QSharedData>
#include <QtGlobal>

class QMailAccountSortKeyPrivate;

class QTOPIAMAIL_EXPORT QMailAccountSortKey
{
public:
    enum Property
    {
        Id,
        Name,
        MessageType,
        EmailAddress
    };

public:
    QMailAccountSortKey();
    explicit QMailAccountSortKey(Property p, Qt::SortOrder order = Qt::AscendingOrder);
    QMailAccountSortKey(const QMailAccountSortKey& other);
    virtual ~QMailAccountSortKey();

    QMailAccountSortKey operator&(const QMailAccountSortKey& other) const;
    QMailAccountSortKey& operator&=(const QMailAccountSortKey& other);

    bool operator==(const QMailAccountSortKey& other) const;
    bool operator !=(const QMailAccountSortKey& other) const;

    QMailAccountSortKey& operator=(const QMailAccountSortKey& other);

    bool isEmpty() const;

private:
    friend class QMailStore;
    friend class QMailStorePrivate;

    QSharedDataPointer<QMailAccountSortKeyPrivate> d;
};

#endif

