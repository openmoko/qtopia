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



#ifndef SmtpClient_H
#define SmtpClient_H

#include <qtcpsocket.h>
#include <qstring.h>
#include <qobject.h>
#include <qtextstream.h>
#include <qstringlist.h>
#include <qlist.h>

#include "account.h"
#include "client.h"

#ifdef SMTPAUTH
#include <qtsslsocket.h>
#endif

struct RawEmail
{
        QString from;
        QStringList to;
//         QString body;
        Email* mail;
};

class SmtpClient: public Client
{
        Q_OBJECT

public:
        SmtpClient();
        ~SmtpClient();
        void newConnection();
        void addMail(QString from, QString subject, QStringList to, QString body);
    int addMail(Email* mail);
        void setAccount(MailAccount *_account);

signals:
    void errorOccurred(int, QString &);
        void updateStatus(const QString &);
        void transferredSize(int);
        void mailSent(int);

public slots:
    void sent(qint64);
        void errorHandling(int, QString msg);
    void socketError(QAbstractSocket::SocketError);

protected slots:
        void connectionEstablished();
        void incomingData();
        void authenticate();
#ifdef SMTPAUTH
    void certCheckDone(QtSslSocket::VerifyResult result,
                       bool hostNameMatched,
                       const QString& description);
#endif
private:
        void createSocket();
        void doSend();
#ifdef SMTPAUTH
        QString _toBase64(const QString& in) const;
        void _switchSecure();
#endif

private:
        QTcpSocket *socket, *socketAuthenticate;
        QTextStream *stream, *streamLogin;
        MailAccount *account;



#ifdef SMTPAUTH
        enum transferStatus
        {
                Init,Auth,AuthUser,AuthPass,StartTLS,TLS,Login,Pass,From,Recv,MRcv,Data,Body,Quit,Done
        };
        QtSslSocket* _secureSocket;
    QTcpSocket* _plainSocket;
#else
        enum transferStatus
        {
                Init, Login, Pass, From, Recv, MRcv, Data, Body, Quit, Done
        };
#endif
        int status, sentSize;
        QList<RawEmail> mailList;
        QList<RawEmail>::Iterator mailItr;
        bool sending, authenticating, success;
        QStringList::Iterator it;
};

#endif
