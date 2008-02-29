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

#include "popclient.h"
#include "emailhandler.h"
#include "common.h"
#include "longstream.h"
#include <qtopialog.h>

PopClient::PopClient()
{
#ifndef SMTPAUTH
    createSocket();
#else
    secureSocket = 0;
    socket = 0;
    stream = 0;
#endif
    receiving = false;
    headerLimit = 0;
    preview = false;
    d = new LongStream();
}

PopClient::~PopClient()
{
    delete d;
    delete stream;
    delete socket;
}

void PopClient::createSocket()
{
#ifdef SMTPAUTH
    if (account->mailEncryption() != MailAccount::Encrypt_NONE) {
        if (secureSocket)
            return;
        secureSocket = new QtSslSocket(QtSslSocket::Client, this);
        secureSocket->setReadBufferSize( 65536 );
        secureSocket->setObjectName("popClient-secure");
        secureSocket->setPathToCACertDir(QtMail::sslCertsPath());
        connect(secureSocket,SIGNAL(connectionVerificationDone(QtSslSocket::VerifyResult,bool,const QString&)),
                SLOT(certCheckDone(QtSslSocket::VerifyResult,bool,const QString&)));
        if (socket) {
            delete stream;
            socket->deleteLater();
        }
        socket = secureSocket;
    } else {
        if (secureSocket) {
            secureSocket->deleteLater();
            delete stream;
            secureSocket = 0;
            socket = 0;
        }
        if (socket)
            return;
        socket = new QTcpSocket(this);
        socket->setReadBufferSize( 65536 );
        socket->setObjectName("popClient");
    }
#else
    socket = new QTcpSocket(this);
    socket->setReadBufferSize( 65536 );
    socket->setObjectName("popClient");
#endif
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
            SLOT(socketError(QAbstractSocket::SocketError)));
    connect(socket, SIGNAL(connected()), this, SLOT(connectionEstablished()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(incomingData()));
    stream = new QTextStream(socket);
    stream->setCodec("UTF-8");
}

#ifdef SMTPAUTH
void PopClient::certCheckDone(QtSslSocket::VerifyResult result, bool hostnameMatch, const QString& msg)
{
    Q_UNUSED(hostnameMatch)
    if(result == QtSslSocket::VerifyOk)
        return;
    qWarning(("SSL cert check failed: " + msg).toLatin1());
    errorHandling(ErrLoginFailed, "");
}
#endif

void PopClient::newConnection()
{
    if (receiving) {
        qWarning("socket in use, connection refused");
        return;
    }

    if ( account->mailServer().isEmpty() ) {
        status = Exit;
        emit mailTransferred(0);
        return;
    }

#ifdef SMTPAUTH
    createSocket();
#endif

    status = Init;
    uidlList.clear();
    uniqueUidlList.clear();
    unresolvedUidl.clear();
    sizeList.clear();
    receiving = true;
    selected = false;
    awaitingData = false;
    newMessages = 0;
    mailDropSize = 0;
    messageCount = 0;
    internalId = QUuid();
    deleteList.clear();

    emit updateStatus(tr("DNS lookup"));
    socket->connectToHost(account->mailServer(), account->mailPort() );
}

void PopClient::setAccount(MailAccount *_account)
{
    account = _account;
    lastUidl = account->getUidlList();
}

void PopClient::headersOnly(bool headers, int limit)
{
    preview = headers;
    headerLimit = limit;
}

void PopClient::setSelectedMails(MailList *list, bool connected)
{
    selected = true;
    mailList = list;

    messageCount = 0;
    mailDropSize = 0;
    newMessages = 0;

    if (connected) {
        status = Retr;
        incomingData();
    }
}

void PopClient::connectionEstablished()
{
#ifdef QTOPIA_PHONE
    emit updateStatus(tr("Connected"));
#else
    emit updateStatus(tr("Connection established"));
#endif
#ifdef SMTPAUTH
    if(secureSocket)
        qLog(Messaging) << "PopClient: Secure connection established";
#endif
}

void PopClient::errorHandling(int status, QString msg)
{
    if ( receiving ) {
        receiving = false;
        account->setUidlList(lastUidl);
        socket->close();
        emit updateStatus(tr("Error occurred"));
        emit errorOccurred(status, msg);
    }
}

void PopClient::socketError(QAbstractSocket::SocketError status)
{
    QString msg = tr("Error occurred");
    socket->close();
    receiving = false;
    emit updateStatus(tr("Error occurred"));
    emit errorOccurred(static_cast<int>(status), msg);
}

void PopClient::quit()
{
    if ( status == Exit ) {
        emit mailTransferred(0);
    } else {
        status = Quit;
        incomingData();
    }
}

// new implementation, need to store uids etc..
void PopClient::incomingData()
{
    QString response, temp;

    if ( (status != Dele) && (status != Quit) )
        response = socket->readLine();

    if (status == Init) {
        emit updateStatus(tr("Logging in"));
        *stream << "USER " << account->mailUserName() << "\r\n" << flush;
        status = Pass;
    } else if (status == Pass) {
        if (response[0] != '+') {
            errorHandling(ErrLoginFailed, "");
            return;
        }
        *stream << "PASS " << account->mailPassword() << "\r\n" << flush;
        status = Uidl;
    } else if (status == Uidl) {
        if (response[0] != '+') {
            errorHandling(ErrLoginFailed, "");
            return;
        }

        status = Guidl;
        awaitingData = false;
        *stream << "UIDL\r\n" << flush;
        return;
    } else if (status == Guidl) {           //get list of uidls

        //means first time in, response should be "+Ok"
        if (! awaitingData) {
            if ( response[0] != '+' ) {
                errorHandling(ErrUnknownResponse, response);
                return;
            }

            awaitingData = true;
            if ( socket->canReadLine() ) {
                response = socket->readLine();
            } else {
                return;
            }
        }
        uidlList.append( response.mid(0, response.length() - 2) );
        while ( socket->canReadLine() ) {
            response = socket->readLine();
            uidlList.append( response.mid(0, response.length() - 2) );
        }
        if (response == ".\r\n") {
            uidlList.removeLast();
            status = List;
        } else {
            return;          //more data incoming
        }
    }

    if (status == List) {
        *stream << "LIST\r\n" << flush;
        awaitingData = false;
        status = Size;
        return;
    }

    if (status == Size) {
        //means first time in, response should be "+Ok"
        if (!awaitingData) {
            if ( response[0] != '+' ) {
                errorHandling(ErrUnknownResponse, response);
                return;
            }

            awaitingData = true;
            if ( socket->canReadLine() )
                response = socket->readLine();
            else return;
        }
        sizeList.append( response.mid(0, response.length() - 2) );
        while ( socket->canReadLine() ) {
            response = socket->readLine();
            sizeList.append( response.mid(0, response.length() - 2) );
        }
        if (response == ".\r\n") {
            sizeList.removeLast();
            status = Retr;
            uidlIntegrityCheck();
        } else {
            return;          //more data incoming
        }
    }

    if (status == Retr) {

        msgNum = nextMsgServerPos();
        if (msgNum != -1) {

            if (selected) {
                // Need twice the size of the mail free, 10kb margin of safety
                const int minFree = mailList->currentSize() * 2 + 1024*10;
                if (!LongStream::freeSpace( "", minFree )) {
                    errorHandling(ErrFileSystemFull, LongStream::errorMessage( "\n" ));
                    status = Exit;
                    return;
                }
            }

            temp.setNum(msgNum);
            if (!selected)
                emit updateStatus(tr("Retrieving %1").arg(temp));
            else
                emit updateStatus(tr("Completing %1").arg(temp));

            if (!preview || mailSize <= headerLimit) {
                *stream << "RETR " << msgNum << "\r\n" << flush;
            } else {                                //only header
                *stream << "TOP " << msgNum << " 0\r\n" << flush;
            }

            status = Ignore;
            return;
        } else {
            status = Acks;
        }
    }

    if (status == Ignore) {
        if (response[0] == '+') {
            message = "";
            d->reset();
            status = Read;
            if (!socket->canReadLine())    //sync. problems
                return;
            response = socket->readLine();
        } else errorHandling(ErrUnknownResponse, response);
    }

    if (status == Read) {
        message += response;
        QString rn = QString::fromLatin1( "\r\n" );
        QString n = QString::fromLatin1( "\n" );
        QtMail::replace( response, rn, n );
        d->append( response );
        if (d->status() == LongStream::OutOfSpace) {
            errorHandling(ErrFileSystemFull, LongStream::errorMessage( "\n" ));
            return;
        }

        while ( socket->canReadLine()) {
            response = socket->readLine();
            message += response;
            message = message.right( 100 );
            QtMail::replace( response, rn, n );
            d->append( response );
            if (d->status() == LongStream::OutOfSpace) {
                errorHandling(ErrFileSystemFull, LongStream::errorMessage( "\n" ));
                return;
            }
        }
        message = message.right( 100 ); // 100 > 5 need at least 5 chars
        emit downloadedSize(mailDropSize + d->length());

        int x = message.indexOf("\r\n.\r\n",-5);
        if (x == -1)
            return;

        createMail();

        // in case user jumps out, don't dowwload message on next entry
        if (preview) {
            lastUidl.append(msgUidl);
            account->setUidlList(lastUidl);
            mailDropSize += headerLimit;
        } else {
            mailDropSize += mailSize;
        }

        newMessages++;

        if(!preview || (preview && mailSize <= headerLimit) && account->deleteMail())
            account->deleteMsg(msgUidl,0);

        status = Retr;
        incomingData();         //remember: recursive
        return;
    }

    if (status == Acks) {
        //ok, delete mail, await user input
        if ( account->deleteMail() ) {
            status = Dele;
            awaitingData = false;
            deleteList = account->msgToDelete();
            if ( deleteList.isEmpty() )
                status = Done;
        } else {
            status = Done;
        }
    }

    if (status == Dele) {
        if ( awaitingData ) {
            response = socket->readLine();
            if ( response[0] != '+' ) {
                errorHandling(ErrUnknownResponse, response);
                return;
            }
        } else {
            qWarning( (QString::number( deleteList.count() ) + " messages in mailbox to be deleted").toAscii());
            emit updateStatus(tr("Removing old messages"));
        }

        if ( deleteList.count() == 0 ) {
            status = Done;
        } else {
            QString str = deleteList.first();
            QString strPos = msgPosFromUidl( str );
            deleteList.removeAll( str );
            account->msgDeleted( str, "" );

            if ( !strPos.isEmpty() ) {
                awaitingData = true;
                qWarning(("deleting message at pos: " + strPos ).toAscii());
                *stream << "DELE " << strPos << "\r\n" << flush;
            } else {
                qWarning(("delete failed on message: " + str ).toAscii());
                incomingData();
                return;
            }
        }
    }

    if (status == Done) {
        if ( preview ) {
            emit mailTransferred(newMessages);
            return;
        }

        status = Quit;
    }

    if (status == Quit) {
        qWarning("quit sent");
        status = Exit;
        *stream << "QUIT\r\n" << flush;
        return;
    }

    if (status == Exit) {
        socket->close();        //close regardless
        receiving = false;
        emit updateStatus(tr("Communication finished"));

        if (unresolvedUidl.count() > 0 && !preview) {
            emit unresolvedUidlList(unresolvedUidl);
        }

        account->setUidlList(uidlList);
        emit mailTransferred(newMessages);
    }
}

QString PopClient::getUidl(QString uidl)
{
    return uidl.mid( uidl.indexOf(" ") + 1, uidl.length() );
}

QString PopClient::msgPosFromUidl(QString uidl)
{
    QStringList list = uidlList.filter(uidl);
    if ( list.count() != 1 )        //should be only 1 match
        return QString::null;

    QString str = list.join("").trimmed();
    bool ok;
    uint x = str.mid(0, str.indexOf(" ") ).toUInt(&ok);

    if ( ok ) {
        return QString::number(x);              //found current pos of msg
    } else {
        return QString::null;
    }
}

int PopClient::nextMsgServerPos()
{
    int thisMsg = -1;
    QString *mPtr;

    if (preview) {
        if ( messageCount < (int) uniqueUidlList.count() ) {
            const QString& it = uniqueUidlList.at(messageCount);
            messageCount++;
            thisMsg = ( it.left( it.indexOf(" ") ) ).toInt();
            msgUidl = it;
            internalId = QUuid();
        }
    }

    if (selected) {
        if (messageCount == 0) {
            messageCount = 1;
            mPtr = mailList->first();
        } else {
            mPtr = mailList->next();
        }

        QStringList ref;

        // if requested mail is not on server, try to get
        // a new mail from the list
        while ( (mPtr != NULL) && (ref.count() == 0) ) {
            ref = uidlList.filter(*mPtr);

            if (ref.count() != 1) {
                unresolvedUidl.append(*mPtr);
                mPtr = mailList->next();
            } else {
                const QString& it = ref.first();
                thisMsg = ( it.left( it.indexOf(" ") ) ).toInt();
                msgUidl = it;
            }
        }

        if (ref.count() != 1)           //temporary, what if two? should never happen
            return thisMsg;

        internalId = mailList->currentId();
    }

    getSize(thisMsg);

    return thisMsg;
}

// get the reported server size from stored list
int PopClient::getSize(int pos)
{
    int sizeRef = 0;
    for ( QStringList::Iterator it = sizeList.begin(); it != sizeList.end(); ++it ) {
        sizeRef = ( (*it).left( (*it).indexOf(" ") ) ).toInt();

        if (sizeRef == pos) {
            mailSize = ( (*it).mid( (*it).indexOf(" ") + 1, (*it).length() )).toInt();
            return mailSize;
        }
    }

    return -1;
}

// checks the list of uidls retrieved from the server by
// comparing them against the last known serverlist.
// if everything is ok, then qtmail is a very happy program
void PopClient::uidlIntegrityCheck()
{
    QString str;

    int pos = 1, size;
    QStringList::Iterator itSize = sizeList.begin();
    for (QStringList::Iterator it = uidlList.begin(); it != uidlList.end(); ++it ) {
        size = getSize(pos);
        str.setNum(size);
        itSize++;
        pos++;
    }

    // create list of new entries that should be downloaded
    if (preview) {
        if ( !account->synchronize() ) {
            lastUidl.clear();
        }

        QStringList previousList;
        QString thisUidl;
        for ( QStringList::Iterator it = uidlList.begin(); it != uidlList.end(); ++it ) {
            thisUidl = getUidl(*it);        //strips pos

            if ( (lastUidl.filter(thisUidl)).count() == 0 ) {
                uniqueUidlList.append(*it);
                mailDropSize += headerLimit;
            } else {
                previousList.append(*it);
            }

        }
        lastUidl = previousList;
        emit mailboxSize(mailDropSize);
        messageCount = 0;
        mailDropSize = 0;
    }
}

void PopClient::createMail()
{
    Email mail;
    bool isComplete = ((!preview ) || ((preview) && (mailSize <= headerLimit)));
    mail.setStatus(EFlag_Incoming, true );
    mail.setUuid( internalId );

    LongString ls( d->fileName(), false );
    mail.fromRFC822( ls );
    mail.setStatus(EFlag_Downloaded, isComplete);
    mail.setSize( mailSize );
    mail.setServerUid( msgUidl.mid( msgUidl.indexOf(" ") + 1, msgUidl.length() ) );

    mail.setFromAccount( account->id() );
    mail.setType(MailMessage::Email);
    mail.setFromMailbox("");
    emit newMessage(mail);
    d->reset();
}
