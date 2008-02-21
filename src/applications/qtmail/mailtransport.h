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


#ifndef MailTransport_H
#define MailTransport_H

#include <QObject>
#include <QAbstractSocket>

#include "account.h"

class QString;
class QTimer;
#ifndef QT_NO_OPENSSL
class QSslSocket;
class QSslError;
#endif

class MailTransport : public QObject
{
    Q_OBJECT

public:
    MailTransport(const char* name);
    virtual ~MailTransport();

    // Open a connection to the server of the specified account
    void open(const QMailAccount& account);

    // Open a connection to the specified server
    void open(const QString& url, int port, QMailAccount::EncryptType encryptionType);

#ifndef QT_NO_OPENSSL
    // If connection is not currently encrypted, switch to encrypted mode
    void switchToEncrypted();
#endif

    // Close the current connection
    void close();

    // Access a stream to write to the mail server (must have an open connection)
    QTextStream& stream();

    // Read line-oriented data from the transport (must have an open connection)
    bool canReadLine() const;
    QByteArray readLine(qint64 maxSize = 0);

signals:
    void connected(QMailAccount::EncryptType encryptType);
    void readyRead();
    void bytesWritten(qint64 transmitted);

    void errorOccurred(int status, QString);
    void updateStatus(const QString &);

public slots:
    void errorHandling(int errorCode, QString msg);
    void socketError(QAbstractSocket::SocketError error);

protected slots:
    void connectionEstablished();
    void hostConnectionTimeOut(); 
#ifndef QT_NO_OPENSSL
    void encryptionEstablished();
    void connectionFailed(const QList<QSslError>& errors);
#endif

#ifndef QT_NO_OPENSSL
protected:
    // Override to modify certificate error handling
    virtual bool ignoreCertificateErrors(const QList<QSslError>& errors);
#endif

private:
    void createSocket(QMailAccount::EncryptType encryptType);
    QMailAccount::EncryptType mailEncryption() const;

private:
#ifndef QT_NO_OPENSSL
    typedef QSslSocket SocketType;
#else
    typedef QAbstractSocket SocketType;
#endif

    SocketType *mSocket;
#ifndef QT_NO_OPENSSL
    QMailAccount::EncryptType encryption;
#endif
    QTextStream *mStream;
    const char *mName;
    QTimer *connectToHostTimeOut;
};

#endif
