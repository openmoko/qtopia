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



#ifndef ImapProtocol_H
#define ImapProtocol_H

#include <qstring.h>
#include <qstringlist.h>
#include <qobject.h>
#include <qtimer.h>

#include "account.h"

enum ImapCommand
{
    IMAP_Init = 0,
    IMAP_Login,
    IMAP_Logout,
    IMAP_List,
    IMAP_Select,
    IMAP_UIDSearch,
    IMAP_UIDFetch,
    IMAP_UIDStore,
    IMAP_Expunge,
    IMAP_Full
};

enum MessageFlag
{
    MFlag_Seen      = 0x0001,
    MFlag_Answered  = 0x0002,
    MFlag_Flagged   = 0x0004,
    MFlag_Deleted   = 0x0008,
    MFlag_Draft     = 0x0010,
    MFlag_Recent    = 0x0020,
    MFlag_Unseen    = 0x0040
};

typedef uint MessageFlags;

enum FetchDataItem
{
    F_Rfc822_Size   =   0x0001,
    F_Rfc822_Header =   0x0002,
    F_Rfc822        =   0x0004,
    F_Uid           =   0x0008,
    F_Flags         =   0x0010
};

typedef uint FetchItemFlags;

enum OperationState
{
    OpDone = 0,
    OpFailed,
    OpOk,
    OpNo,
    OpBad
};

class LongStream;
class Email;
class MailTransport;

class ImapProtocol: public QObject
{
    Q_OBJECT

public:
    ImapProtocol();
    ~ImapProtocol();

    bool open(const QMailAccount& account);
    void close();
    bool connected() { return _connected; };

    /*  Valid in non-authenticated state only    */
    void login(QString user, QString password);

    /* Valid in authenticated state only    */
    void list(QString reference, QString mailbox);
    void select(QString mailbox);

    /*  Valid in Selected state only */
    void uidSearch(uint from, uint to, MessageFlags flags = 0);
    void uidSearch(MessageFlags flags);
    void uidFetch(QString from, QString to, FetchItemFlags items);
    void uidStore(QString uid, MessageFlags flags);
    void expunge();

    /*  Internal commands (stored from selected mailbox)    */
    QString selected();
    int exists();
    int recent();
    QString mailboxUid();
    QString flags();
    QStringList mailboxUidList();

    /*  Valid in all states */
    void logout();

    QString lastError() { return _lastError; };

    static QString token(QString str, QChar c1, QChar c2, int *index);

signals:
    void mailboxListed(QString &flags, QString &delimiter, QString &name);
    void messageFetched(QMailMessage& mail);
    void downloadSize(int);
    void nonexistentMessage(const QString& uuid);

    void finished(ImapCommand &, OperationState &);
    void updateStatus(const QString &);
    void connectionError(int status, QString msg);

protected slots:
    void connected(QMailAccount::EncryptType encryptType);
    void errorHandling(int status, QString msg);
    void incomingData();
    void parseFetch();

private:
    void nextAction();

    QString newCommandId();
    QString commandId(QString in);
    OperationState commandResponse(QString in);
    void sendCommand(QString cmd);

    void parseSelect();
    void parseFetchAll();
    void parseUid();
    void parseChange();
    void parseList(QString in);

    void createMail(const QByteArray& msg, QString& uid, int size, uint flags);

private:
    MailTransport *transport;

    ImapCommand status;
    OperationState operationState;
    bool _connected;
    MessageFlags messageFlags;
    FetchItemFlags dataItems;

    /*  Associated with the Mailbox */
    QString _name;
    int _exists, _recent;
    QString _flags, _mailboxUid;
    QStringList uidList;

    QStringList requests;
    QStringList errorList;
    LongStream *d;
    int requestCount, internalId;
    int messageLength;

    QString _lastError;
    QString response;
    int read;
    QTimer incomingDataTimer;
    QTimer parseFetchTimer;
    bool firstParseFetch;
    QString fetchUid;

    static const int MAX_LINES = 30;
};

#endif
