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



#ifndef PopClient_H
#define PopClient_H

#include <qstring.h>
#include <qstringlist.h>
#include <qobject.h>
#include <qlist.h>

#include "maillist.h"
#include "account.h"
#include "client.h"

class LongStream;
class MailTransport;

class PopClient: public Client
{
    Q_OBJECT

public:
    PopClient();
    ~PopClient();
    void newConnection();
    void setAccount(MailAccount *_account);
    void headersOnly(bool headers, int limit);
    void setSelectedMails(MailList *list, bool connected);
    void quit();
    void checkForNewMessages();
    int newMessageCount();

signals:
    void newMessage(const QMailMessage&);
    void unresolvedUidlList(QStringList &);
    void mailTransferred(int);
    void mailboxSize(int);
    void downloadedSize(int);
    void allMessagesReceived();

public slots:
    void errorHandling(int, QString msg);

protected slots:
    void incomingData();

private:
    int nextMsgServerPos();
    QString getUidl(QString uidl);
    QString msgPosFromUidl(QString uidl);
    int getSize(int pos);
    void uidlIntegrityCheck();
    void createMail();

private:
    enum TransferStatus
    {
            Init, Pass, Stat, Mcnt, Read, List, Size, Retr, Acks,
            Quit, Done, Ignore, Dele, Rset, Uidl, Guidl, Exit
    };

    MailAccount *account;
    TransferStatus status;
    int messageCount, newMessages, mailSize, headerLimit;
    int msgNum, mailDropSize;
    QMailId internalId;
    bool receiving, preview, selected;
    bool awaitingData;
    QString message;
    MailList *mailList;

    QString msgUidl;
    QStringList uidlList, uniqueUidlList, unresolvedUidl;
    QStringList sizeList;
    QStringList lastUidl;
    QStringList deleteList;
    LongStream *d;

    MailTransport *transport;
};

#endif
