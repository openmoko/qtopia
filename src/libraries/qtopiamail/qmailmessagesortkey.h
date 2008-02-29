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

#ifndef __QMAILMESSAGESORTKEY_H
#define __QMAILMESSAGESORTKEY_H

#include <qtopiaglobal.h>
#include <QSharedData>
#include <QtGlobal>

class QMailMessageSortKeyPrivate;

class QTOPIAMAIL_EXPORT QMailMessageSortKey
{
public:
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
    QMailMessageSortKey(const Property& p, const Qt::SortOrder& order = Qt::AscendingOrder);
    QMailMessageSortKey(const QMailMessageSortKey& other);
    virtual ~QMailMessageSortKey();

    QMailMessageSortKey operator&(const QMailMessageSortKey& other) const;
    QMailMessageSortKey& operator&=(const QMailMessageSortKey& other);

    bool operator==(const QMailMessageSortKey& other) const;
    bool operator !=(const QMailMessageSortKey& other) const;

    QMailMessageSortKey& operator=(const QMailMessageSortKey& other);

private:
    QMailMessageSortKey();

private:
	friend class QMailStore;
	friend class QMailStorePrivate;	
    
private:
    QSharedDataPointer<QMailMessageSortKeyPrivate> d;

};

#endif
