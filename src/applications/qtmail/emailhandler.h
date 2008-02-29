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

#ifndef EmailHandler_H
#define EmailHandler_H

#include "account.h"
#include "client.h"
#include "smtpclient.h"
#ifndef QTOPIA_NO_SMS
#include "smsclient.h"
#endif
#include "popclient.h"
#include "imapclient.h"
#include "maillist.h"

#include <qobject.h>
#include <qstring.h>
#include <qlist.h>
#include <qmap.h>
#include <qstringlist.h>

const int ErrUnknownResponse = 1001;
const int ErrLoginFailed = 1002;
const int ErrCancel = 1003;
const int ErrFileSystemFull = 1004;
const int ErrNonexistentMessage = 1005;

class MmsClient;
class QDSActionRequest;

class EmailHandler : public QObject
{
    Q_OBJECT

public:
    EmailHandler();
    ~EmailHandler();

    enum TransmissionType {
        Unknown = 0,
        Receiving = 0x01,
        Sending = 0x02,
        AsyncArrival = 0x04,
        AsyncDeletion = 0x08
    };

    void connectClient(Client *client, int type, QString sigName);
    void setMailAccount(MailAccount *account);
    void setSmtpAccount(MailAccount *account);
    void setSmsAccount(MailAccount *account);
    void setMmsAccount(MailAccount *account);
    void sendMail(QList<QMailMessage>& mailList);
    void getMailHeaders();
    void getMailByList(MailList *mailList, bool newConnection);
    void cancel();
    void popQuit();
    void acceptMail(const QMailMessage& mail, bool closeAfterSend = false);
    void rejectMail(const QMailMessage& mail);
    int unreceivedSmsCount();
    int unreadSmsCount();
    Client* clientFromAccount(const MailAccount *account) const;
    MailAccount* accountFromClient(const Client *client) const;
    bool smsReadyToDelete() const;
    void synchroniseClients();
    int newMessageCount();
    void pushMmsMessage(const QDSActionRequest& request);

signals:
    void mailSent(int);
    void transmissionCompleted();
    void smtpError(int, QString &);
    void popError(int, QString &);
    void smsError(int, QString &);
    void mmsError(int, QString &);
    void mailArrived(const QMailMessage&);
    void unresolvedUidlList(QString &, QStringList &);
    void updateReceiveStatus(const Client*, const QString&);
    void updateSendStatus(const Client*, const QString&);
    void mailTransferred(int);
    void mailboxSize(int);
    void downloadedSize(int);
    void transferredSize(int);
    void mailSendSize(int);
    void serverFolders();
    void failedList(QStringList &);
    void allMessagesReceived();
    void nonexistentMessage(const QMailId&);

public slots:
    void unresolvedUidl(QStringList &);
    void mailRead(const QMailMessage&);
    void receiveStatusChange(const QString&);
    void sendStatusChange(const QString&);
    void sendProgress(const QMailId&, uint percentage);
    void messageProcessed(const QMailId&);

private slots:
    void messagesReceived();

// Define this to report client signal emissions:
//#define DEBUG_CLIENT_SIGNALS
#ifdef DEBUG_CLIENT_SIGNALS
    void reportErrorOccurred(int n, QString& s);
    void reportMailSent(int n);
    void reportTransmissionCompleted();
    void reportNonexistentMessage(const QMailId& id);
    void reportUpdateStatus(const QString& s);
    void reportNewMessage(const QMailMessage& m);
    void reportAllMessagesReceived();
    void reportMailTransferred(int n);
    void reportDownloadedSize(int n);
    void reportMailboxSize(int n);
    void reportSendProgress(const QMailId&, uint percentage);
    void reportMessageProcessed(const QMailId& id);
#endif

private:
    MailAccount *mailAccount;
    MailAccount *smtpAccount;
    MailAccount *smsAccount;
    MailAccount *mmsAccount;
    SmtpClient *smtpClient;
#ifndef QTOPIA_NO_SMS
    SmsClient *smsClient;
#endif
#ifndef QTOPIA_NO_MMS
    MmsClient *mmsClient;
#endif
    PopClient *popClient;
    ImapClient *imapClient;
    bool receiving;
    QList<const Client*> unsynchronised;
    QMap<QMailId, uint> sendSize;
    uint totalSendSize;
};

#endif
