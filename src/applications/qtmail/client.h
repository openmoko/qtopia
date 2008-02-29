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

#ifndef Client_H
#define Client_H

#include <qobject.h>

class MailAccount;
class MailList;

class Client : public QObject
{
    Q_OBJECT

public:
    Client();
    virtual ~Client();
    virtual void setAccount(MailAccount *_account);
    virtual void headersOnly(bool headers, int limit);
    virtual void newConnection();
    virtual void setSelectedMails(MailList *list, bool connected);
    virtual void quit();
    virtual bool hasDeleteImmediately() const;
    virtual void deleteImmediately(const QString& serverUid);
    virtual void resetNewMailCount();

signals:
    void errorOccurred(int, QString &);
    void mailSent(int);
    void updateStatus(const QString &);
    void mailTransferred(int);
    void unresolvedUidlList(QStringList &);
    void serverFolders();
    void downloadedSize(int);
    void mailboxSize(int);
    void transferredSize(int);
    void failedList(QStringList &);
    void deviceReady();
};

#endif
