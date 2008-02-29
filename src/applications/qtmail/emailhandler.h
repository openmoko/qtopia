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

#ifndef EmailHandler_H
#define EmailHandler_H

#include "email.h"
#include "account.h"
#include "client.h"
#include "smtpclient.h"
#ifdef QTOPIA_PHONE
#include "smsclient.h"
#endif
#include "popclient.h"
#include "imapclient.h"
#include "maillist.h"

#include <qobject.h>
#include <qstring.h>
#include <qlist.h>
#include <qstringlist.h>

const int ErrUnknownResponse = 1001;
const int ErrLoginFailed = 1002;
const int ErrCancel = 1003;
const int ErrFileSystemFull = 1004;

class MmsClient;

class EmailHandler : public QObject
{
    Q_OBJECT

public:
    EmailHandler();
    ~EmailHandler();

    enum TransmissionType {
        Unknown = 0,
        Receiving = 0x01,
        Sending = 0x02
    };

    void connectClient(Client *client, int type, QString sigName);
    void setMailAccount(MailAccount *account);
    void setSmtpAccount(MailAccount *account);
    void setSmsAccount(MailAccount *account);
    void setMmsAccount(MailAccount *account);
    void sendMail(QList<Email*>* mailList);
    void getMailHeaders();
    void getMailByList(MailList *mailList, bool newConnection);
    void cancel();
    void popQuit();
    void acceptMail(const Email &mail);
    void rejectMail(const Email &mail);
    int unreceivedSmsCount();
    Client* clientFromAccount(MailAccount *account);
    bool smsReadyToDelete() const;

signals:
    void mailSent(int);
    void smtpError(int, QString &);
    void popError(int, QString &);
    void smsError(int, QString &);
    void mmsError(int, QString &);
    void mailArrived(const Email &);
    void unresolvedUidlList(QString &, QStringList &);
    void updatePopStatus(const QString &);
    void updateSendingStatus(const QString &);
    void mailTransferred(int);
    void mailboxSize(int);
    void downloadedSize(int);
    void transferredSize(int);
    void mailSendSize(int);
    void serverFolders();
    void failedList(QStringList &);

public slots:
    void unresolvedUidl(QStringList &);
    void mailRead(Email*);

private:
    MailAccount *mailAccount;
    MailAccount *smtpAccount;
    MailAccount *smsAccount;
    MailAccount *mmsAccount;
    SmtpClient *smtpClient;
#ifdef QTOPIA_PHONE
#ifndef QTOPIA_NO_SMS
    SmsClient *smsClient;
#endif
#ifndef QTOPIA_NO_MMS
    MmsClient *mmsClient;
#endif
#endif
    PopClient *popClient;
    ImapClient *imapClient;
    bool receiving;
};

#endif
