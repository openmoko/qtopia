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



#include "smtpclient.h"
#include "emailhandler.h"
#include <qtopiaapplication.h>
#include <QTextCodec>

#ifdef SMTPAUTH
#include "common.h"
#endif

//helper class to normalize newlines on the fly

class NewLineFilter : public QTextCodec
{
public:
    NewLineFilter():QTextCodec(),_state(0){};
    ~NewLineFilter(){};
    int mibEnum() const{return -1;};
    QByteArray name() const{return "NewLineFilter";};
    QByteArray convertFromUnicode ( const QChar * input, int number, ConverterState * state ) const;
    QString convertToUnicode ( const char * chars, int len, ConverterState * state ) const;
private:
    mutable int _state; //ugh
};

QByteArray NewLineFilter::convertFromUnicode(const QChar* input, int number, ConverterState* state) const
{
    Q_UNUSED(state);
    QByteArray result;
    for (int i = 0; i < number; ++i)
    {
        QChar current = input[i];
        if (current == '\n')
        {
            if (_state == 0)
                _state = 1;
            else if (_state == 2)
            {
                result.append('.');
                _state = 0;
            }
            result.append('\r');
        }
        else if (current == '.' && _state == 1)
            _state = 2;
        else _state = 0;

        result.append(current.toAscii());
    }
    return result;
}

QString NewLineFilter::convertToUnicode(const char* chars, int len, ConverterState* state) const
{
    Q_UNUSED(state);
    return QByteArray(chars,len);
}

SmtpClient::SmtpClient()
{
#ifndef SMTPAUTH
    createSocket();
#else
    socket = 0;
    stream = 0;
    _secureSocket = 0;
    _plainSocket = 0;
#endif

    socketAuthenticate = new QTcpSocket(this);
    socketAuthenticate->setObjectName("smtpClient-Authenticate");
    streamLogin = new QTextStream(socketAuthenticate);
    connect(socketAuthenticate, SIGNAL(error(QAbstractSocket::SocketError)),
            SLOT(socketError(QAbstractSocket::SocketError)));
    connect(socketAuthenticate, SIGNAL(readyRead()), this, SLOT(authenticate()));

    sending = false;
    authenticating = false;
    account = 0;
}

void SmtpClient::createSocket()
{

#ifdef SMTPAUTH
    if (account->smtpEncryption() == MailAccount::Encrypt_SSL)
    {

        //previous == none
        if (socket && (socket != _secureSocket))
        {
            socket->deleteLater();
            delete stream;
            socket = 0;
        }
        //previous == tls
        if (_plainSocket)
        {
            _plainSocket->deleteLater();
            _plainSocket = 0;
            _secureSocket->deleteLater();
            _secureSocket = 0;
            delete stream;
        }
        //previous == ssl
        if (_secureSocket)
            return;

        _secureSocket = new QtSslSocket(QtSslSocket::Client,this);
        _secureSocket->setObjectName("smtpClient-secure");
        _secureSocket->setPathToCACertDir(QtMail::sslCertsPath());
        connect(_secureSocket,SIGNAL(connectionVerificationDone(QtSslSocket::VerifyResult,
                                     bool, const QString&)),SLOT(certCheckDone(QtSslSocket::VerifyResult,bool,const QString&)));
        socket = _secureSocket;
    }
    else
    {

        //previous == ssl or tls
        if (socket && (socket == _secureSocket))
        {
            _secureSocket->deleteLater();
            delete stream;
            _secureSocket = 0;
            socket = 0;
        }
        //previous == tls
        if (_plainSocket)
        {
            _plainSocket->deleteLater();
            _plainSocket = 0;
        }
        //previous == none
        if (socket)
            return;
        socket = new QTcpSocket(this);
        socket->setObjectName("smtpClient");
    }
#else
    socket = new QTcpSocket(this);
    socket->setObjectName("smtpClient");
#endif
    stream = new QTextStream(socket);

    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
            SLOT(socketError(QAbstractSocket::SocketError)));
    connect(socket, SIGNAL(connected()), this, SLOT(connectionEstablished()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(incomingData()));
    connect(socket, SIGNAL(bytesWritten(qint64)), this, SLOT(sent(qint64)) );
}

SmtpClient::~SmtpClient()
{
    delete socketAuthenticate;
    delete stream;
    delete socket;
#ifdef SMTPAUTH
    delete _plainSocket;
#endif
}

void SmtpClient::setAccount(MailAccount *_account)
{
    account = _account;
}

void SmtpClient::newConnection()
{
    if (sending) {
        qWarning("socket in use, connection refused");
        return;
    }

#ifdef SMTPAUTH
    createSocket();
#endif

    if ( !account || account->smtpServer().isEmpty() ) {
        qWarning("Tried to send an email without a account and/or SMTP server");
        status = Done;
        emit mailSent(-1);
        return;
    }

    if ( account->smtpServer().isEmpty() ) {
        status = Done;
        emit mailSent(-1);
        return;
    }

    // authenticate with POP first
#ifdef SMTPAUTH
    if ( !account->mailServer().isEmpty() && (account->smtpAuthentication() == MailAccount::Auth_NONE)) {
#else
    if (!account->mailServer().isEmpty()) {
#endif
        authenticating = true;
        status = Init;
        sending = true;
        emit updateStatus(tr("DNS lookup"));
        socketAuthenticate->connectToHost( account->mailServer(), account->mailPort() );
    } else {
        doSend();
    }

}

void SmtpClient::doSend()
{
    status = Init;
    sending = true;
    authenticating = false;

    emit updateStatus(tr("DNS lookup"));
    sentSize = 0;
#ifdef SMTPAUTH
    if (_secureSocket) //connectToHost not virtual
        _secureSocket->connectToHost(account->smtpServer(),account->smtpPort());
    else
#endif
        socket->connectToHost(account->smtpServer(), account->smtpPort() );
}


#ifdef SMTPAUTH

QString SmtpClient::_toBase64(const QString& in) const
{
    QByteArray temp(in.toAscii().data(),in.length());
    return MailMessage::encodeBase64(temp);
}

void SmtpClient::_switchSecure()
{
    _secureSocket = new QtSslSocket(QtSslSocket::Client,this);
    _secureSocket->setObjectName("smtpClient-secure");
    _secureSocket->setPathToCACertDir(QtMail::sslCertsPath());
    _secureSocket->setSocket(socket);
    connect(_secureSocket,SIGNAL(connectionVerificationDone(QtSslSocket::VerifyResult,
                                 bool, const QString &)),SLOT(certCheckDone(QtSslSocket::VerifyResult,bool,const QString&)));
    _plainSocket = socket;
    socket = _secureSocket;
    stream = new QTextStream(socket);
    connect(socket, SIGNAL(error( QAbstractSocket::SocketError)),
            SLOT(socketError(QAbstractSocket::SocketError)));
    connect(socket, SIGNAL(connected()), this, SLOT(connectionEstablished()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(incomingData()));
    connect(socket, SIGNAL(bytesWritten(qint64)), this, SLOT(sent(qint64)) );
    _secureSocket->sslConnect();
}

#endif

int SmtpClient::addMail(Email* mail)
{
    int pos, endPos;
    int mailSize = 0;

    //only send to mail addresses

    QStringList addressList = mail->mailRecipients();

    for (QStringList::Iterator it = addressList.begin();
            it != addressList.end(); ++it) {
        if ( (pos = (*it).indexOf('<', 0 , Qt::CaseInsensitive)) != -1) {

            pos++;
            endPos = (*it).indexOf('>', pos, Qt::CaseInsensitive);
            *it = (*it).mid(pos, endPos - pos).trimmed();
        }
    }

    if ( account && account->useSig() )
        mail->setPlainTextBody( mail->plainTextBody() + "\n" + account->sig() );

    QString userName;
    if (account)
        userName = "<" + account->emailAddress() + ">";

    //create a raw mail

    //TODO get rid of this, lock removed mails so they don't get deleted


    RawEmail rawmail;
    if ( mail->replyTo().isEmpty() )
        rawmail.from = "<" + mail->fromEmail() + ">";
    else
        rawmail.from = userName;
    rawmail.to = addressList;
    rawmail.mail = mail;
    mailList.append(rawmail);

    return mailSize;
}

void SmtpClient::connectionEstablished()
{
#ifdef QTOPIA_PHONE
    emit updateStatus(tr("Connected"));
#else
    emit updateStatus(tr("Connection established"));
#endif

#ifdef SMTPAUTH
    if (status == TLS)
    {
        *stream << "EHLO qtmail\r\n" << flush;
        if (account->smtpAuthentication() != MailAccount::Auth_NONE)
            status = Auth;
        else
            status = From;
    }
#endif

}
#ifdef SMTPAUTH
void SmtpClient::certCheckDone(QtSslSocket::VerifyResult result,
                               bool hostNameMatched,
                               const QString& description)
{
    Q_UNUSED(hostNameMatched)
    if (result == QtSslSocket::VerifyOk)
        return;
    qWarning(("SSL cert check failed: " + description).toAscii());
    errorHandling(ErrLoginFailed, "");
}
#endif

void SmtpClient::errorHandling(int status, QString msg)
{
    if ( !sending )
        return;

    if (authenticating) {
        qWarning("login failed");
        //login failed, try sending anyway
        socketAuthenticate->close();

        if (status == ErrCancel) {
            mailList.clear();
            sending = false;
            emit errorOccurred(status, msg);
            return;
        } else {
            doSend();
        }
    } else {
        socket->close();

        //brutal, but I can't see any other way to delete
        //the outgoing buffer
#ifndef SMTPAUTH
        delete stream;
        delete socket;
        createSocket();
#endif
        mailList.clear();
        sending = false;
        emit errorOccurred(status, msg);
        qWarning("socket closed");
    }
}

void SmtpClient::socketError(QAbstractSocket::SocketError status)
{
    if (authenticating) {
        //scoket fail, pop may be down, but try sending anyway
        socketAuthenticate->close();
        doSend();
        return;
    } else {
        socket->close();
    }

    QString msg = tr("Error occurred");
    mailList.clear();
    sending = false;
    emit updateStatus(msg);
    emit errorOccurred(static_cast<int>(status), msg);
}

void SmtpClient::sent(qint64 size)
{
    if (status == From || status == Quit) {
        sentSize += size;
        emit transferredSize(sentSize);
    }
}

void SmtpClient::authenticate()
{
    QString response;

    response = socketAuthenticate->readLine();

    if ( account->accountType() == MailAccount::IMAP ) {
        if (status == Init ) {
            status = Done;
            *streamLogin << "A01 LOGIN " + account->mailUserName() + " " + account->mailPassword() + "\r\n" << flush;
            return;
        } else if ( status == Done ) {
            QString rsp = response.mid(3, response.indexOf(" ") ).trimmed();
            if ( rsp.toUpper() != "OK") {
                errorHandling(ErrLoginFailed, "");
                return;
            }

            status = Quit;
            *streamLogin << "A02 LOGOUT\r\n" << flush;
            return;
        } else if ( status == Quit ) {
            socketAuthenticate->close();
            doSend();
            return;
        }
    }

    if (status == Init) {
        status = Pass;
        *streamLogin << "USER " << account->mailUserName() << "\r\n" <<flush;
        return;
    } else if (status == Pass) {
        if (response[0] != '+') {
            errorHandling(ErrLoginFailed, "");
            return;
        }

        status = Done;
        *streamLogin << "PASS " << account->mailPassword() << "\r\n" << flush;
        return;
    } else if (status == Done) {
        if (response[0] != '+') {
            errorHandling(ErrLoginFailed, "");
            return;
        }

        status = Quit;
        *streamLogin << "QUIT\r\n" << flush;
        return;
    } else if ( status == Quit ) {
        socketAuthenticate->close();
        doSend();
    }
}

void SmtpClient::incomingData()
{
    QString response;
    QString line;


    if (!socket->canReadLine())
        return;

    do
    {
        line = socket->readLine();
        response += line;
    }
    while (socket->canReadLine() && line.length() != 0);


    switch (status) {
    case Init:  {
        if (response[0] == '2') {
            status = From;
            mailItr = mailList.begin();
#ifdef SMTPAUTH
            if (account->smtpAuthentication() != MailAccount::Auth_NONE ||
                    account->smtpEncryption() == MailAccount::Encrypt_TLS)
            {
                *stream << "EHLO qtmail\r\n" << flush;
                if (account->smtpEncryption() == MailAccount::Encrypt_TLS)
                    status = StartTLS;
                else
                    status = Auth;
            }
            else
#endif
                *stream << "HELO qtmail\r\n" << flush;
        } else errorHandling(ErrUnknownResponse, response);
        break;
    }
#ifdef SMTPAUTH
    case StartTLS:
    {
        if (line[0] == '2')
        {
            *stream << "STARTTLS\r\n" << flush;
            status = TLS;
        } else
            errorHandling(ErrUnknownResponse,response);
        break;
    }
    case TLS:
    {
        if (line[0] == '2')
        {
            _switchSecure();
        }
        else
            errorHandling(ErrUnknownResponse,response);
        break;
    }

    case Auth:
    {
        if (line[0] == '2')
        {
            if (account->smtpAuthentication() == MailAccount::Auth_LOGIN)
            {
                *stream << "AUTH LOGIN \r\n" << flush;
                status = AuthUser;
            }
            else if (account->smtpAuthentication() == MailAccount::Auth_PLAIN)
            {
                QString temp = account->smtpUsername() + '\0' + account->smtpUsername() + '\0' + account->smtpPassword();
                temp = _toBase64(temp);
                *stream << "AUTH PLAIN " << temp << "\r\n" << flush;
                status = From;
            }
        }
        else
            errorHandling(ErrUnknownResponse, response);

        break;
    }

    case AuthUser:
    {

        if (line[0] == '3')
        {
            *stream << _toBase64(account->smtpUsername()) << "\r\n" << flush;
            status = AuthPass;
        }
        else
            errorHandling(ErrUnknownResponse, response);
        break;
    }
    case AuthPass:
    {
        if (line[0] == '3')
        {
            *stream << _toBase64(account->smtpPassword()) << "\r\n" << flush;
            status = From;
        }
        else
            errorHandling(ErrUnknownResponse, response);
        break;
    }
#endif

    case From:  {
        if (response[0] == '2') {
            *stream << "MAIL FROM: " << mailItr->from << "\r\n" << flush;
            status = Recv;
        } else errorHandling(ErrUnknownResponse, response);
        break;
    }
    case Recv:  {
        if (response[0] == '2') {
            it = mailItr->to.begin();
            if (it == mailItr->to.end())
                errorHandling(ErrUnknownResponse, "no recipients");
            *stream << "RCPT TO: <" << *it << ">\r\n" << flush;
            status = MRcv;
        } else errorHandling(ErrUnknownResponse, response);
        break;
    }
    case MRcv:  {
        if (response[0] == '2') {
            it++;
            if ( it != mailItr->to.end() ) {
                *stream << "RCPT TO: <" << *it << ">\r\n" << flush;
                break;
            } else  {
                status = Data;
            }
        } else {
            errorHandling(ErrUnknownResponse, response);
            break;
        }
    }
    case Data:  {
        if (response[0] == '2') {
            *stream << "DATA\r\n" << flush;
            status = Body;
            emit updateStatus(tr( "Sending: %1").arg(mailItr->mail->subject()) );
        } else errorHandling(ErrUnknownResponse, response);
        break;
    }
    case Body:  {
        if (response[0] == '3' || response[0] == '2') {
            NewLineFilter* f = new NewLineFilter();
            stream->setCodec(f);
            mailItr->mail->encodeMail(*stream,false,true);
            stream->setCodec("UTF-8");
            *stream << "\r\n.\r\n" << flush;
            //*stream << mailItr->body << "\r\n.\r\n" << flush;
            mailItr++;
            if (mailItr != mailList.end()) {
                status = From;
            } else {
                status = Quit;
            }
        } else
            errorHandling(ErrUnknownResponse, response);
        break;
    }
    case Quit:  {
        if ( response[0] == '2' || response[0] == '3' ) {
            *stream << "QUIT\r\n" << flush;
            status = Done;
            int count = mailList.count();
            emit updateStatus( tr("Sent %1 messages").arg(count) );
            mailList.clear();
            sending = false;
            socket->close();
            emit mailSent(count);
            emit mailSent(-1);
        } else errorHandling(ErrUnknownResponse, response);
        break;
    }
    }

}
