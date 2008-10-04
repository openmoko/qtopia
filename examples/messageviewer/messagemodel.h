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

#ifndef MESSAGEMODEL_H
#define MESSAGEMODEL_H

#include <QContactModel>
#include <QStandardItemModel>

class QContact;
class QMailMessageId;

class MessageModel : public QStandardItemModel
{
    Q_OBJECT

public:
    explicit MessageModel(QObject* parent = 0);
    virtual ~MessageModel();

    void setContact(const QContact&);

    bool isEmpty() const;

    QMailMessageId messageId(const QModelIndex& index);
};

#endif
