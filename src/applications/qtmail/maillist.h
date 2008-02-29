/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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



#ifndef MAILLIST_H
#define MAILLIST_H

#include <qobject.h>
#include <qstring.h>
#include <qlist.h>

#include <quuid.h>

struct dList{
    QString serverId;
    uint size;
    QUuid internalId;
    QString fromBox;
};

class MailList : public QObject
{
    Q_OBJECT

public:
    void clear();
    int count();
    int size();
    int currentSize();
    QUuid currentId();
    QString currentMailbox();

    QString* first();
    QString* next();
    void sizeInsert(QString serverId, uint size, QUuid id, QString box);
    void append(QString serverId, uint size, QUuid id, QString box);
    void moveFront(QString serverId);
    bool remove(QString serverId);

private:
    QList<dList*> sortedList;
    uint currentPos;
};

#endif
