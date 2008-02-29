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


#include <qapplication.h>
#include <QTemporaryFile>
#include "imapprotocol.h"
#include "common.h"
#include "email.h"
#include "longstream.h"

ImapProtocol::ImapProtocol()
{
    _connected = false;
    read = 0;
    firstParseFetch = true;
    d = new LongStream();
    socket = 0;
    stream = 0;
#ifdef SMTPAUTH
    secureSocket = 0;
#endif
}

ImapProtocol::~ImapProtocol()
{
    delete socket;
    delete stream;
    delete d;
}

bool ImapProtocol::open( QString url, int port )
{
    if ( _connected ) {
        qWarning("socket in use, connection refused");
        return false;
    }
#ifdef SMTPAUTH
    if (secureSocket) {
        secureSocket->deleteLater();
        delete stream;
        secureSocket = 0;
        socket = 0;
    }
#endif

    if (!socket) {
      socket = new QTcpSocket( this );
      socket->setReadBufferSize( 65536 );
      socket->setObjectName( "imapProtocol" );
      connect( socket, SIGNAL(error(QAbstractSocket::SocketError)),
               SLOT(socketError(QAbstractSocket::SocketError)));
      connect( socket, SIGNAL(connected()), this,
               SLOT(connectionEstablished()));
      connect( socket, SIGNAL(readyRead()), this, SLOT(incomingData()));
      connect( &incomingDataTimer, SIGNAL(timeout()), this,
               SLOT(incomingData()));
      connect( &parseFetchTimer, SIGNAL(timeout()), this, SLOT(parseFetch()));
      stream = new QTextStream( socket );
    }

    status = IMAP_Init;

    errorList.clear();
    _connected = true;

    requestCount = 0;
    requests.clear();
    d->reset();

    socket->connectToHost( url, port );
    return true;
}

#ifdef SMTPAUTH
bool ImapProtocol::openSecure( QString url, int port, bool useTLS )
{
    if ( _connected ) {
        qWarning("socket in use, connection refused");
        return false;
    }

    if (socket && !secureSocket)
    {
        socket->deleteLater();
        delete stream;
        socket = 0;
    }

    //if useTLS is enabled then enable starttls for the connection

    if (!secureSocket) {
        if (!useTLS) { //SSL connection
            secureSocket = new QtSslSocket( QtSslSocket::Client, this );
            secureSocket->setReadBufferSize( 65536 );
            secureSocket->setObjectName( "imapProtocol-secure" );
            secureSocket->setPathToCACertDir( QtMail::sslCertsPath() );
            connect( secureSocket,
                     SIGNAL(connectionVerificationDone(QtSslSocket::VerifyResult, bool, const QString&)),
                     SLOT(certCheckDone(QtSslSocket::VerifyResult, bool, const QString&)));
            socket = secureSocket;
            connect( socket, SIGNAL(error(QAbstractSocket::SocketError)),
                     SLOT(socketError(QAbstractSocket::SocketError)));
            connect( socket, SIGNAL(connected()),
                     this, SLOT(connectionEstablished()));
            connect( socket, SIGNAL(readyRead()),
                     this, SLOT(incomingData()));
            connect( &incomingDataTimer, SIGNAL(timeout()),
                     this, SLOT(incomingData()));
            connect( &parseFetchTimer, SIGNAL(timeout()),
                     this, SLOT(parseFetch()));
            stream = new QTextStream( socket );
        } else {
            //TODO add support for starttls command
        }
    }

    status = IMAP_Init;

    errorList.clear();
    _connected = true;

    requestCount = 0;
    requests.clear();
    d->reset();

    secureSocket->connectToHost( url, port );
    return true;
}


void ImapProtocol::certCheckDone( QtSslSocket::VerifyResult result, bool hostMatched, const QString& msg)
{
    Q_UNUSED(hostMatched)
    if (result == QtSslSocket::VerifyOk)
        return;

    qWarning( ("SSL cert check failed: " + msg).toAscii() );
    socket->close();
    _connected = false;
    emit connectionError( static_cast<int>(result) );
}

#endif

void ImapProtocol::close()
{
    _connected = false;
    _name = "";
    socket->close();
}

int ImapProtocol::exists()
{
    return _exists;
}

int ImapProtocol::recent()
{
    return _recent;
}

QString ImapProtocol:: mailboxUid()
{
    return _mailboxUid;
}

QString ImapProtocol::flags()
{
    return _flags;
}

QStringList ImapProtocol::mailboxUidList()
{
    return uidList;
}

QString ImapProtocol::selected()
{
    return _name;
}

/*  Type ignored for now    */
void ImapProtocol::login( QString user, QString password )
{
    QString cmd = "LOGIN " + user + " " + password + "\r\n";
    status = IMAP_Login;
    sendCommand( cmd );
}

void ImapProtocol::logout()
{
    QString cmd = "LOGOUT\r\n";
    status = IMAP_Logout;
    sendCommand( cmd );
}

void ImapProtocol::list( QString reference, QString mailbox )
{
    QString cmd = "LIST \"" + reference + "\" " + mailbox + "\r\n";
    status = IMAP_List;
    sendCommand( cmd );
}

void ImapProtocol::select( QString mailbox )
{
    QString cmd = "SELECT " + quoteString(mailbox) + "\r\n";
    status = IMAP_Select;
    _name = mailbox;
    sendCommand(cmd);
}

void ImapProtocol::uidSearch( uint from, uint to, MessageFlags flags )
{
    QString str = "";
    if ( flags & MFlag_Recent )
        str += " RECENT";
    if (flags & MFlag_Deleted)
        str += " DELETED";
    if (flags & MFlag_Answered)
        str += " ANSWERED";
    if (flags & MFlag_Flagged)
        str += " FLAGGED";
    if (flags & MFlag_Seen)
        str += "  SEEN";
    if (flags & MFlag_Unseen)
        str += " UNSEEN";
    if (flags & MFlag_Draft)
        str += " DRAFT";

    QString cmd = QString("UID SEARCH %1:%2%3\r\n").arg(from).arg(to).arg(str);
    status = IMAP_UIDSearch;
    sendCommand(cmd);
}

void ImapProtocol::uidSearch( MessageFlags flags )
{
    QString str = "";
    if ( flags & MFlag_Recent )
        str += " RECENT";
    if (flags & MFlag_Deleted)
        str += " DELETED";
    if (flags & MFlag_Answered)
        str += " ANSWERED";
    if (flags & MFlag_Flagged)
        str += " FLAGGED";
    if (flags & MFlag_Seen)
        str += "  SEEN";
    if (flags & MFlag_Unseen)
        str += " UNSEEN";
    if (flags & MFlag_Draft)
        str += " DRAFT";

    QString cmd = QString( "UID SEARCH%1\r\n" ).arg( str );
    status = IMAP_UIDSearch;
    sendCommand( cmd );
}

void ImapProtocol::uidFetch( QString from, QString to, FetchItemFlags items )
{
    dataItems = items;

    QString flags = "(FLAGS";
    if (dataItems & F_UID)
        flags += " UID";
    if (dataItems & F_RFC822_SIZE)
        flags += " RFC822.SIZE";
    if (dataItems & F_RFC822_HEADER)
        flags += " RFC822.HEADER";
    if (dataItems & F_RFC822)
        flags += " RFC822";

    flags += ")";

    QString cmd = QString( "UID FETCH %1:%2 %3\r\n" ).arg( from ).arg( to ).arg( flags );
    status = IMAP_UIDFetch;

    sendCommand( cmd );
}

/*  Note that \Recent flag is ignored as only the server is allowed
    to set/unset that flag      */
void ImapProtocol::uidStore( QString uid, MessageFlags flags )
{
    QString str = "FLAGS (";
    if (flags & MFlag_Deleted) {
        str += "\\Deleted "; // No tr
        uidList.removeAll(uid);
    }
    if (flags & MFlag_Answered)
        str += "\\Answered "; // No tr
    if (flags & MFlag_Flagged)
        str += "\\Flagged "; // No tr
    if (flags & MFlag_Seen)
        str += "\\Seen "; // No tr
    if (flags & MFlag_Draft)
        str += "\\Draft "; // No tr
    str = str.trimmed() + ")";

    QString cmd = QString( "UID STORE %1 %2\r\n" ).arg( uid ).arg( str );

    status = IMAP_UIDStore;
    sendCommand(cmd);
}

void ImapProtocol::expunge()
{
    QString cmd = "EXPUNGE\r\n";
    status = IMAP_Expunge;
    sendCommand( cmd );
}

void ImapProtocol::connectionEstablished()
{
   // qWarning( "Connection established");
#ifdef SMTPAUTH
    if(secureSocket)
        qLog(Messaging) << "ImapProtocol: Secure connection established";
#endif
    mailDropSize = 0;
}

void ImapProtocol::socketError( QAbstractSocket::SocketError status )
{
    _connected = false;
    socket->close();
    if (this->status != IMAP_Logout)
        emit connectionError(static_cast<int>(status));
}

void ImapProtocol::sendCommand( QString cmd )
{
    QString command = newCommandId() + " " + cmd;

    requests.append( command );
    d->reset();

//    qWarning("sending command: " + command);

    *stream << command << flush;
}

void ImapProtocol::incomingData()
{
    int readLines = 0;
    while (socket->canReadLine()) {
        response = socket->readLine();
        readLines++;
        read += response.length();

        if (status != IMAP_Init) {
            d->append( response );
            if (d->status() == LongStream::OutOfSpace) {
                operationState = OpFailed;
                _lastError += LongStream::errorMessage( "\n" );
                status = IMAP_Full;
                emit finished(status, operationState);
                response = "";
                read = 0;
                return;
            }
        }

        if ((status == IMAP_UIDFetch) && (dataItems & F_RFC822)) {
            mailDropSize += response.length();
        }
        if (readLines > MAX_LINES) {
            incomingDataTimer.start(0);
            return;
        }
    }

    incomingDataTimer.stop();

    if (status == IMAP_Init) {
        operationState = OpOk;
        emit finished(status, operationState);
        response = "";
        read = 0;
        return;
    }

    if (status == IMAP_Logout) {
        socket->close();
        _connected = false;
        d->reset();
        operationState = OpOk;
        _name = "";
        emit finished(status, operationState);
        response = "";
        read = 0;
        return;
    }

    if ((status == IMAP_UIDFetch) && (dataItems & F_RFC822))
        emit downloadSize( mailDropSize );

    /* Applies to all functions below   */
    if (!response.startsWith( commandId( requests.at(requests.count() - 1 )))) {
        response = "";
        read = 0;
        return;
    }

    if ((operationState = commandResponse( response )) != OpOk) {
        // The client decides whether the error is critical or not
        qWarning( response.toAscii() );
        // tr string from server - this seems ambitious
        _lastError = tr( response.toAscii() );
        if (status == IMAP_UIDSearch)
            _lastError += QLatin1String("\n") + tr( "This server does not provide a complete "
                              "IMAP4rev1 implementation." );
        emit finished(status, operationState);
        response = "";
        read = 0;
        return;
    }

    if (status == IMAP_Login) {
        emit finished(status, operationState);
        response = "";
        read = 0;
        return;
    }

    if (status == IMAP_List) {
        for (QString str = d->first(); str != QString::null; str = d->next()) {
            if (str.startsWith("* LIST"))
                parseList( str.mid(7) );
        }

        emit finished(status, operationState);
        response = "";
        read = 0;
        return;
    }

    if (status == IMAP_Select) {
        parseSelect();

        emit finished(status, operationState);
        response = "";
        read = 0;
        return;
    }

    if (status == IMAP_UIDSearch) {
        parseUid();

        emit finished(status, operationState);
        response = "";
        read = 0;
        return;
    }

    if (status == IMAP_UIDFetch) {
        //temporary for now
        if (dataItems & F_RFC822_HEADER) {
            parseFetch();
        } else {
            parseFetchAll();
            emit finished(status, operationState);
            response = "";
            read = 0;
        }

        return;
    }

    if (status == IMAP_UIDStore) {
        emit finished(status, operationState);
        response = "";
        read = 0;
        return;
    }

    if (status == IMAP_Expunge) {
        emit finished(status, operationState);
        response = "";
        read = 0;
        return;
    }

/*
    if (status == Exit) {
        socket->close();
        receiving = false;
        serverData.clear();
        emit updateStatus(tr("Communication finished"));
//      if (unresolvedUidl.count() > 0 && !preview) {
//          emit unresolvedUidlList(unresolvedUidl);
//      }
        emit mailTransferred(0);
        if (errorList.count() > 0)
            emit failedList(errorList);
    }
*/
    response = "";
    read = 0;
}

QString ImapProtocol::newCommandId()
{
    QString id, out;

    requestCount++;
    id.setNum( requestCount );
    out = "a";
    out = out.leftJustified( 4 - id.length(), '0' );
    out += id;
    return out;
}

QString ImapProtocol::commandId( QString in )
{
    int pos = in.indexOf(' ');
    if (pos == -1)
        return "";

    return in.left( pos ).trimmed();
}

OperationState ImapProtocol::commandResponse( QString in )
{
    QString old = in;
    int start = in.indexOf( ' ' );
    start = in.indexOf( ' ', start );
    int stop = in.indexOf( ' ', start + 1 );
    if (start == -1 || stop == -1) {
        qWarning(("could not parse command response: " + in).toAscii());
        return OpFailed;
    }

    in = in.mid( start, stop - start ).trimmed().toUpper();
    OperationState state = OpFailed;

    if (in == "OK")
        state = OpOk;
    if (in == "NO")
        state = OpNo;
    if (in == "BAD")
        state = OpBad;

    return state;
}


void ImapProtocol::parseList( QString in )
{
    QString flags, name, delimiter;
    int pos, index = 0;
    bool noSelect;

    flags = token( in, '(', ')', &index );
    noSelect = (flags.indexOf("noselect", 0, Qt::CaseInsensitive) != -1);
    if (noSelect)
        return;

    delimiter = token( in, ' ', ' ', &index );
    pos = 0;
    if (token(delimiter, '"', '"', &pos) != QString::null) {
        pos = 0;
        delimiter = token( delimiter, '"', '"', &pos );
    }

    index--;    //to point back to previous => () NIL "INBOX"
    name = token( in, ' ', '\n', &index ).trimmed();
    pos = 0;
    if (token( name, '"', '"', &pos ) != QString::null) {
        pos = 0;
        name = token( name, '"', '"', &pos );
    }

    emit mailboxListed( flags, delimiter, name );
}

void ImapProtocol::parseSelect()
{
    int start;
    bool result;
    QString str, temp;

    // reset all attributes in case some are not reported
    _exists = 0;
    _recent = 0;
    _flags = "";
    _mailboxUid = "";
    for (str = d->first(); str != QString::null; str = d->next()) {

        if (str.indexOf("EXISTS", 0) != -1) {
            start = 0;
            temp = token(str, ' ', ' ', &start);
            _exists =  temp.toInt(&result);
            if (!result)
                _exists = 0;
        } else if (str.indexOf("RECENT", 0) != -1) {
            start = 0;
            temp = token( str, ' ', ' ', &start );
            _recent = temp.toInt( &result );
            if (!result)
                _recent = 0;
        } else if (str.startsWith("* FLAGS")) {
            start = 0;
            _flags = token( str, '(', ')', &start );
        } else if (str.indexOf("UIDVALIDITY", 0) != -1) {
            start = 0;
            temp = token( str, '[', ']', &start );
            _mailboxUid = temp.mid( 12 );
        }
    }
}

void ImapProtocol::parseFetch()
{
    int start, endMsg;
    QString str, uid, msg, size;
    MessageFlags flags = 0;

    if (firstParseFetch) {
        firstParseFetch = false;
        str = d->first();
    } else {
        str = d->current();
    }

    if (str != QString::null) {
        if (str.startsWith( "* " )) {
            if ((start = str.indexOf( "UID", 0 )) != -1) {
                uid = token( str, ' ', ' ', &start );
                //get flags as well
            }
            if ((start = str.indexOf( "RFC822.SIZE", 0 )) != -1) {
                size = token( str, ' ', ' ', &start );
            }

            if (str.indexOf("\\Seen") > -1)
                flags |= MFlag_Seen;
            if (str.indexOf("\\Answered") > -1)
                flags |= MFlag_Answered;
            if (str.indexOf("\\Flagged") > -1)
                flags |= MFlag_Flagged;
            if (str.indexOf("\\Deleted") > -1)
                flags |= MFlag_Deleted;
            if (str.indexOf("\\Draft") > -1)
                flags |= MFlag_Draft;
            if (str.indexOf("\\Recent") > -1)
                flags |= MFlag_Recent;

            msg = "";
            str = d->next();
            while (!str.startsWith("* ") && str != QString::null) {
                msg += str;
                str = d->next();
            }

            if (str.toUpper().startsWith("* NO")) {
                qWarning( ("fetch failed: " + str).toAscii() );
                flags = 0;
            } else {
                endMsg = msg.indexOf( "\n\n" );
                if (endMsg != -1)
                    msg.truncate( (uint)endMsg);

                //to work with pop standard
                endMsg = msg.indexOf( "\n.\n", -3 );
                if (endMsg == -1)
                    msg += "\n.\n";

                createMail( false, msg, uid, size.toInt(), flags );

                flags = 0;
                // don't freeze up the pda when we're very busy
                parseFetchTimer.start( 0 );
                return;
            }
        } else {
            str = d->next();
        }
    } else {
        firstParseFetch = true;
        parseFetchTimer.stop();
        ImapCommand status = IMAP_UIDFetch;
        emit finished(status, operationState);
        response = "";
        read = 0;
    }
}

void ImapProtocol::parseFetchAll()
{
    int start;
    int msgSize = 0;
    QString uid, size;
    bool result;
    MessageFlags flags = 0;

    QString str = d->first();
    if (str == QString::null) {
        qWarning( "not a valid message" );
        return;
    }

    if (str.startsWith( "* " )) {
        //get flags as well
        if ((start = str.indexOf( "UID", 0 )) != -1) {
            uid = token( str, ' ', ' ', &start );
        }
        if ((start = str.indexOf( "RFC822.SIZE", 0 )) != -1) {
            size = token( str, ' ', ' ', &start );
            msgSize = size.toUInt( &result );
            if (!result)
                msgSize = 0;
        }

        if (str.indexOf("\\Seen") > -1)
            flags |= MFlag_Seen;
        if (str.indexOf("\\Answered") > -1)
            flags |= MFlag_Answered;
        if (str.indexOf("\\Flagged") > -1)
              flags |= MFlag_Flagged;
        if (str.indexOf("\\Deleted") > -1)
              flags |= MFlag_Deleted;
        if (str.indexOf("\\Draft") > -1)
            flags |= MFlag_Draft;
        if (str.indexOf("\\Recent") > -1)
            flags |= MFlag_Recent;
    }

    // Make a copy of the downloaded file
    QString fileName;
    QTemporaryFile *tmpFile;
    QTextStream *ts;
    QString tmpName( LongStream::tempDir() + QLatin1String( "/qtmail" ) );
    tmpFile = new QTemporaryFile( tmpName + QLatin1String( ".XXXXXX" ));
    tmpFile->setAutoRemove( false );
    tmpFile->open(); // todo error checking
    fileName = tmpFile->fileName();
    ts = new QTextStream( tmpFile );
    ts->setCodec( "UTF-8" ); // Mail should be 7bit ascii
    str = d->next();
    while (str != QString::null) {
        *ts << str << flush; //todo error checking - out of disk
        str = d->next();
    }
    tmpFile->close();
    delete ts;
    delete tmpFile;

    // If necessary truncate the copy to remove spurious imap server chatter,
    // e.g. remove "junk" lines:   )\r\n* Ax FETCH ...
    // then unconditionally append "\n.\n" to it.
    LongString ls( fileName, false );
    int pos = ls.toQByteArray().lastIndexOf( "\n)\n", -1 );
    QFile mailFile( fileName );
    QTextStream *mts;

    if (pos > 0)
        mailFile.resize( pos + 1 );
    mailFile.open( QIODevice::Append | QIODevice::Text );// todo error checking
    mts = new QTextStream( &mailFile );
    mts->setCodec( "UTF-8" ); // Mail should be 7bit ascii
    *mts << "\n.\n" << flush;
    mailFile.close();
    delete mts;

    // Clear the download file minimizing simultaneous copies on the filesystem
    d->reset();

    // Create the mail and emit the relevant signal
    createMail( true, fileName, uid, msgSize, flags );
}

void ImapProtocol::parseUid()
{
    int index;
    QString str, temp;

    uidList.clear();
    for (str = d->first(); str != QString::null; str = d->next()) {
        if (str.startsWith("* SEARCH")) {
            index = 7;
            while ((temp = token( str, ' ', ' ', &index )) != QString::null) {
                uidList.append( temp );
                index--;
            }
            temp = token( str, ' ', '\n', &index );
            if (temp != QString::null)
                uidList.append( temp );
        }
    }
}

QString ImapProtocol::token( QString str, QChar c1, QChar c2, int *index )
{
    int start, stop;
    start = str.indexOf( c1, *index, Qt::CaseInsensitive );
    if (start == -1)
        return QString::null;
    stop = str.indexOf( c2, ++start, Qt::CaseInsensitive );
    if (stop == -1)
        return QString::null;

    *index = stop + 1;
    return str.mid( start, stop - start );
}


/* Adds "" to mailbox names with spaces and not to mailboxes without.
   Some IMAP servers require this methology.  */
QString ImapProtocol::quoteString( QString name )
{
    if (name.indexOf(' ', 0) != -1)
        return "\"" + name + "\"";

    return name;
}

QString ImapProtocol::unquoteString( QString name )
{
    QString str = name.simplified();
    if (str[0] == '\"')
        str = str.right( str.length() - 1 );
    if (str[(int)str.length() - 1] == '\"')
        str = str.left( str.length() - 1 );

    return str.trimmed();
}

void ImapProtocol::createMail( bool isFile, QString& msg, QString& id, int size, uint flags )
{
    Email mail;
    mail.setStatus( EFlag_Incoming, true );
    if (flags & MFlag_Seen) {
        mail.setStatus( EFlag_IMAP_Seen, true );
    }
    if (flags & MFlag_Answered)
        mail.setStatus( EFlag_Replied, true );

    if (isFile) {
        LongString ls( msg );
        mail.fromRFC822( ls );
    } else {
        QString str = msg;
        mail.fromRFC822( str );
    }
    mail.setSize( size );
    mail.setServerUid( id.mid( id.indexOf( " " ) + 1, id.length() ));
    mail.setType( MailMessage::Email );

    emit messageFetched(mail);
}
