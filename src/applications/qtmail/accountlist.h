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



#ifndef ACCOUNTLIST_H
#define ACCOUNTLIST_H

#include <qobject.h>
#include <qlist.h>
#include <qstringlist.h>

#include <qtopiaglobal.h>

#include "account.h"

class QTOPIAMAIL_EXPORT AccountList : public QObject
{
    Q_OBJECT

public:
    AccountList(QObject *parent=0, const char *name=0);

    QListIterator<QMailAccount*> accountIterator();
    int count();

    //temp while converting
    QMailAccount* at(int x);
    void append(QMailAccount *a);
    void remove(int pos);
    void remove(QMailAccount* const a);

    void readAccounts();
    void saveAccounts();

    uint defaultMailServerCount();
    QMailAccount* defaultMailServer();
    void setDefaultMailServer(QMailAccount *account);

    QMailAccount* getSmtpRef();
    QMailAccount* getPopRefByAccount(QString user);
    QMailAccount* getAccountById(QString id);
    QMailAccount* getSmtpRefByMail(QString email);
    QStringList emailAccounts();
    QString getUserName();

signals:
    void accountListChanged(); //not currently used
    void checkAccount(int);

public slots:
    void intervalCheck(QMailAccount*);

private:
    QList<QMailAccount*> list;
};

#endif
