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



#ifndef ImapClient_H
#define ImapClient_H

#include <qstring.h>
#include <qstringlist.h>
#include <qobject.h>
#include <qlist.h>



#include "maillist.h"
#include "account.h"
#include "imapprotocol.h"
#include "client.h"

class ImapClient: public Client
{
    Q_OBJECT

public:
    ImapClient();
    ~ImapClient();
    void newConnection();
    void setAccount(MailAccount *_account);
    void setSelectedMails(MailList *list, bool connected);
    void quit();
    void checkForNewMessages();
    int newMessageCount();

signals:
    void serverFolders();
    void newMessage(const QMailMessage&);
    void mailTransferred(int);
    void downloadedSize(int);
    void failedList(QStringList &);
    void allMessagesReceived();
    void nonexistentMessage(const QMailId& id);

public slots:
    void errorHandling(int, QString msg);

protected slots:
    void operationDone(ImapCommand &, OperationState &);
    void mailboxListed(QString &, QString &, QString &);
    void messageFetched(QMailMessage& mail);
    void nonexistentMessage(const QString& uid);

private:
    void removeDeletedMailboxes();
    bool nextMailbox();
    void handleSelect();
    void handleUid();
    void handleUidFetch();
    void handleSearch();

    bool messagesToDelete();
    void setNextDeleted();
    void fetchNextMail();

private:
    ImapProtocol client;
    MailAccount *account;
    MailList *mailList;
    QString msgUidl;
    enum transferStatus
    {
        Init, Fetch, Rfc822
    };
    enum SearchStatus
    {
        All,Seen,Unseen
    };
    SearchStatus _searchStatus;
    QStringList _mailboxUidList;

    int status;
    int messageCount, mailSize, mailDropSize;
    bool selected;

    QStringList mailboxNames;
    QMailId internalId;
    QStringList uniqueUidList, unresolvedUid, delList;
    Mailbox *currentBox;
    uint atCurrentBox;
};

#endif
