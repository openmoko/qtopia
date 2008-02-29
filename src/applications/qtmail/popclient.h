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



#ifndef PopClient_H
#define PopClient_H

#include <qtcpsocket.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qobject.h>
#include <qtextstream.h>
#include <qlist.h>

#include "maillist.h"
#include "account.h"
#include "client.h"
#include "email.h"

#ifdef SMTPAUTH
#include <qtsslsocket.h>
#include "common.h"
#endif

#include <quuid.h>

class LongStream;

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

signals:
    void newMessage(const Email&);
    void unresolvedUidlList(QStringList &);
    void errorOccurred(int status, QString &);
    void updateStatus(const QString &);
    void mailTransferred(int);
    void mailboxSize(int);
    void downloadedSize(int);

public slots:
    void errorHandling(int, QString msg);
    void socketError(QAbstractSocket::SocketError);

protected slots:
    void connectionEstablished();
    void incomingData();
#ifdef SMTPAUTH
    void certCheckDone(QtSslSocket::VerifyResult,bool,const QString&);
#endif

private:
    int nextMsgServerPos();
    QString getUidl(QString uidl);
    QString msgPosFromUidl(QString uidl);
    int getSize(int pos);
    void uidlIntegrityCheck();
    void createSocket();
    void createMail();
private:
    QTcpSocket *socket;
#ifdef SMTPAUTH
    QtSslSocket* secureSocket;
#endif
    QTextStream *stream;
    MailAccount *account;
    enum transferStatus
    {
            Init, Pass, Stat, Mcnt, Read, List, Size, Retr, Acks,
            Quit, Done, Ignore, Dele, Rset, Uidl, Guidl, Exit
    };
    int status;
    int messageCount, newMessages, mailSize, headerLimit;
    int msgNum, mailDropSize;
    QUuid internalId;
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
};

#endif
