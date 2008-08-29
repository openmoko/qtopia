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

#ifndef __QMAILFOLDERSORTKEY_H
#define __QMAILFOLDERSORTKEY_H

#include <qtopiaglobal.h>
#include <QSharedData>
#include <QtGlobal>

class QMailFolderSortKeyPrivate;

class QTOPIAMAIL_EXPORT QMailFolderSortKey
{
public:
    enum Property
    {
        Id,
        Name,
        ParentId
    };

public:
    QMailFolderSortKey();
    QMailFolderSortKey(const Property& p, const Qt::SortOrder& order = Qt::AscendingOrder);
    QMailFolderSortKey(const QMailFolderSortKey& other);
    virtual ~QMailFolderSortKey();

    QMailFolderSortKey operator&(const QMailFolderSortKey& other) const;
    QMailFolderSortKey& operator&=(const QMailFolderSortKey& other);

    bool operator==(const QMailFolderSortKey& other) const;
    bool operator !=(const QMailFolderSortKey& other) const;

    QMailFolderSortKey& operator=(const QMailFolderSortKey& other);

    bool isEmpty() const;

private:
    friend class QMailStore;
    friend class QMailStorePrivate;

private:
    QSharedDataPointer<QMailFolderSortKeyPrivate> d;

};

#endif
