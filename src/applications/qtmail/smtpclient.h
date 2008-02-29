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



#ifndef SmtpClient_H
#define SmtpClient_H

#include <qstring.h>
#include <qobject.h>
#include <qstringlist.h>
#include <qlist.h>
#include <QMailMessage>

#include "account.h"
#include "client.h"

class MailTransport;

struct RawEmail
{
    QString from;
    QStringList to;
    QMailMessage mail;
};

class SmtpClient: public Client
{
    Q_OBJECT

public:
    SmtpClient();
    ~SmtpClient();
    void newConnection();
    bool addMail(const QMailMessage& mail);
    void setAccount(MailAccount *_account);

signals:
    void mailSent(int);
    void transmissionCompleted();
    void sendProgress(const QMailId&, uint);
    void messageProcessed(const QMailId&);

public slots:
    void sent(qint64);
    void errorHandling(int, QString msg);

protected slots:
    void connected(MailAccount::EncryptType encryptType);
    void incomingData();
    void authenticate();

private:
    void doSend(bool authenticating = false);
#ifndef QT_NO_OPENSSL
    QString _toBase64(const QString& in) const;
#endif

private:
    enum TransferStatus
    {
        Init,
#ifndef QT_NO_OPENSSL
        StartTLS, TLS, Auth, AuthUser, AuthPass,
#endif
        Login, Pass, Done, From, Recv, MRcv, Data, Body, Quit
    };

    MailAccount *account;
    TransferStatus status;
    QList<RawEmail> mailList;
    QList<RawEmail>::Iterator mailItr;
    RawEmail* sendMail;
    uint messageLength;
    uint sentLength;
    bool sending, authenticating, success;
    QStringList::Iterator it;
    MailTransport *transport;
};

#endif
