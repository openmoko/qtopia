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


#ifndef MMSCLIENT_H
#define MMSCLIENT_H

#include <quuid.h>
#include <qlist.h>
#include <qobject.h>
#include "account.h"
#include "client.h"
#include "email.h"

class MailAccount;
class MailList;
class MMSMessage;
class MmsComms;
class QHttp;
class QHttpResponseHeader;
class QHttpRequestHeader;
class QWspPart;

class MmsClient: public Client
{
    Q_OBJECT
public:
    MmsClient();
    ~MmsClient();

    void newConnection();
    void quit();
    void setSelectedMails(MailList *list, bool connected);
    void addMail(const MailMessage &);
    void setAccount(MailAccount *_account);
    void sendNotifyResp(const MailMessage &, const QString &status);
    void resetNewMailCount();

signals:
    void errorOccurred(int, QString &);
    void updateStatus(const QString &);
    void transferredSize(int);
    void mailTransferred(int);
    void mailSent(int);
    void newMessage(const Email&);

public slots:
    void errorHandling(int, QString msg);

private slots:
    void notificationInd(const MMSMessage &msg);
    void deliveryInd(const MMSMessage &msg);
    void sendConf(const MMSMessage &msg);
    void retrieveConf(const MMSMessage &msg, int);
    void statusChange(const QString &status);
    void commsError(int code, const QString &msg);
    void transferSize(int);
    void transfersComplete();

private:
    void sendAcknowledge(const MMSMessage &);
    void sendNextMessage();
    void getNextMessage();
    Email convertToEmail(const MMSMessage &, int size);
    MMSMessage convertToMms(const MailMessage &mail);
    void addField(MMSMessage &mms, const Email &mail, const QString &field);
    QString encodeRecipient(const QString &r);
    QString decodeRecipient(const QString &r);
    QString networkConfig() const;
private:
    MailAccount *account;
    MmsComms *comms;
    MailList *mailList;
    QList<MMSMessage> outgoing;
    QUuid internalId;
    int messagesSent;
    int messagesRecv;
    bool quitRecv;
    static int txnId;
};

#endif
