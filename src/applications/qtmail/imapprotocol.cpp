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

#include "imapprotocol.h"

#include <QApplication>
#include <QTemporaryFile>

#include "mailtransport.h"
#include <longstream_p.h>
#include <longstring_p.h>
#include <QMailMessage>


// Ensure a string is quoted, if required for IMAP transmission
static QString quoteImapString(const QString& input)
{
    // We can't easily catch controls other than those caught by \\s...
    static const QRegExp atomSpecials("[\\(\\)\\{\\s\\*%\\\\\"\\]]");

    // The empty string must be quoted
    if (input.isEmpty())
        return QString("\"\"");

    if (atomSpecials.indexIn(input) == -1)
        return input;
        
    // We need to quote this string because it is not an atom
    QString result(input);

    QString::iterator begin = result.begin(), it = begin;
    while (it != result.end()) {
        // We need to escape any characters specially treated in quotes
        if ((*it) == '\\' || (*it) == '"') {
            int pos = (it - begin);
            result.insert(pos, '\\');
            it = result.begin() + (pos + 1);
        }
        ++it;
    }

    return QMail::quoteString(result);
}


ImapProtocol::ImapProtocol()
{
    _connected = false;
    read = 0;
    firstParseFetch = true;
    transport = 0;
    d = new LongStream();
    connect(&incomingDataTimer, SIGNAL(timeout()),
            this, SLOT(incomingData()));
    connect(&parseFetchTimer, SIGNAL(timeout()),
            this, SLOT(parseFetch()));
}

ImapProtocol::~ImapProtocol()
{
    delete d;
    delete transport;
}

bool ImapProtocol::open( const QMailAccount& account )
{
    if ( _connected ) {
        qLog(IMAP) << "transport in use, connection refused";
        return false;
    }

    status = IMAP_Init;

    errorList.clear();
    _connected = true;

    requestCount = 0;
    requests.clear();
    d->reset();

    if (!transport) {
        transport = new MailTransport("IMAP");

        connect(transport, SIGNAL(updateStatus(QString)),
                this, SIGNAL(updateStatus(QString)));
        connect(transport, SIGNAL(errorOccurred(int,QString)),
                this, SLOT(errorHandling(int,QString)));
        connect(transport, SIGNAL(connected(QMailAccount::EncryptType)),
                this, SLOT(connected(QMailAccount::EncryptType)));
        connect(transport, SIGNAL(readyRead()),
                this, SLOT(incomingData()));
    }

    transport->open( account );

    return true;
}

void ImapProtocol::close()
{
    _connected = false;
    _name = "";
    transport->close();
    parseFetchTimer.stop();
    d->reset();
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
    QString cmd = "LOGIN " + quoteImapString(user) + " " + quoteImapString(password) + "\r\n";
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
    QString cmd = "LIST " + quoteImapString(reference) + " " + quoteImapString(mailbox) + "\r\n";
    status = IMAP_List;
    sendCommand( cmd );
}

void ImapProtocol::select( QString mailbox )
{
    QString cmd = "SELECT " + quoteImapString(mailbox) + "\r\n";
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
    if (dataItems & F_Uid)
        flags += " UID";
    if (dataItems & F_Rfc822_Size)
        flags += " RFC822.SIZE";
    if (dataItems & F_Rfc822_Header)
        flags += " RFC822.HEADER";
    if (dataItems & F_Rfc822)
        flags += " BODY.PEEK[]";

    flags += ")";

    QString cmd = QString( "UID FETCH %1:%2 %3\r\n" ).arg( from ).arg( to ).arg( flags );
    if (from == to)
        fetchUid = from;
    status = IMAP_UIDFetch;
    messageLength = 0;

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

void ImapProtocol::connected(QMailAccount::EncryptType encryptType)
{
#ifndef QT_NO_OPENSSL
    if (encryptType == QMailAccount::Encrypt_NONE)
    {
        // TODO - TLS support not yet added!
    }
#endif
}

void ImapProtocol::errorHandling(int status, QString msg)
{
    _connected = false;
    transport->close();

    if (msg.isEmpty())
        msg = tr("Connection failed");

    if (this->status != IMAP_Logout)
        emit connectionError(status, msg);
}

void ImapProtocol::sendCommand( QString cmd )
{
    QString command = newCommandId() + " " + cmd;

    requests.append( command );
    d->reset();

    transport->stream() << command << flush;

    if (command.length() > 1)
        qLog(IMAP) << "SEND:" << qPrintable(command.left(command.length() - 2));
}

void ImapProtocol::incomingData()
{
    int readLines = 0;
    while (transport->canReadLine()) {
        response = transport->readLine();
        readLines++;
        read += response.length();

        if (response.length() > 1)
            qLog(IMAP) << "RECV:" << qPrintable(response.left(response.length() - 2));

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

        if ((status == IMAP_UIDFetch) && (dataItems & F_Rfc822)) {
            if (!response.startsWith("* "))
                messageLength += response.length();
        }

        if (readLines > MAX_LINES) {
            incomingDataTimer.start(0);
            return;
        }
    }

    incomingDataTimer.stop();

    nextAction();
}

void ImapProtocol::nextAction()
{
    if (status == IMAP_Init) {
        operationState = OpOk;
        emit finished(status, operationState);
        response = "";
        read = 0;
        return;
    }

    if (status == IMAP_Logout) {
        transport->close();
        _connected = false;
        d->reset();
        operationState = OpOk;
        _name = "";
        emit finished(status, operationState);
        response = "";
        read = 0;
        return;
    }

    if ((status == IMAP_UIDFetch) && (dataItems & F_Rfc822))
        emit downloadSize( messageLength );

    /* Applies to all functions below   */
    if (!response.startsWith( commandId( requests.at(requests.count() - 1 )))) {
        response = "";
        read = 0;
        return;
    }

    if ((operationState = commandResponse( response )) != OpOk) {
        // The client decides whether the error is critical or not
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
        if (dataItems & F_Rfc822_Header) {
            parseFetch();
        } else {
            parseFetchAll();
            emit finished(status, operationState);
            response = "";
            read = 0;
        }

        fetchUid = QString();
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
        qLog(IMAP) << qPrintable("could not parse command response: " + in);
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
    static const QByteArray crlfSequence( QMailMessage::CRLF );
    static const QByteArray headerTerminator( crlfSequence + crlfSequence );
    static const QByteArray popTerminator( crlfSequence + '.' + crlfSequence );

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
                qLog(IMAP) << qPrintable("fetch failed: " + str);
                flags = 0;
            } else {
                endMsg = msg.indexOf( headerTerminator );
                if (endMsg != -1)
                    msg.truncate(endMsg);

                //to work with pop standard
                endMsg = msg.indexOf( popTerminator, -popTerminator.length() );
                if (endMsg == -1)
                    msg.append( popTerminator );

                createMail( msg.toLatin1(), uid, size.toInt(), flags );

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

static bool parseFlags(const QString& field, MessageFlags& flags)
{
    QRegExp pattern("FLAGS *\\((.*)\\)");
    pattern.setMinimal(true);

    if (pattern.indexIn(field) == -1)
        return false;

    QString messageFlags = pattern.cap(1);

    flags = 0;
    if (messageFlags.indexOf("\\Seen") != -1)
        flags |= MFlag_Seen;
    if (messageFlags.indexOf("\\Answered") != -1)
        flags |= MFlag_Answered;
    if (messageFlags.indexOf("\\Flagged") != -1)
        flags |= MFlag_Flagged;
    if (messageFlags.indexOf("\\Deleted") != -1)
        flags |= MFlag_Deleted;
    if (messageFlags.indexOf("\\Draft") != -1)
        flags |= MFlag_Draft;
    if (messageFlags.indexOf("\\Recent") != -1)
        flags |= MFlag_Recent;

    return true;
}

void ImapProtocol::parseFetchAll()
{
    QString str = d->first();
    if (str == QString::null) {
        qLog(IMAP) << "not a valid message";
        return;
    }

    if (str.startsWith( "* " )) {
        QString uid, size;
        uint msgSize = 0;

        QRegExp format("UID *(\\d+).*RFC822\\.SIZE *(\\d+)");
        if (format.indexIn(str) != -1) {
            uid = format.cap(1);
            size = format.cap(2);
            msgSize = size.toUInt();
        }

        MessageFlags flags = 0;
        bool flagsParsed(parseFlags(str, flags));

        int bypass = str.length() + 2;  // Account for CRLF

        // Read the body data from the file
        LongString ls(d->fileName());
        
        const QByteArray& orgData(ls.toQByteArray());
        uint remainder(orgData.length() - bypass);
        if (msgSize)
            remainder = qMin(msgSize, remainder);

        QByteArray data(orgData.constData() + bypass, remainder);

        QByteArray trailer;
        if (msgSize == 0) {
            // Find the trailing part of the imap server traffic
            trailer = QByteArray( (QByteArray( QMailMessage::CRLF ) + ')' + QMailMessage::CRLF) );
            int pos = data.lastIndexOf( trailer );
            if (pos != -1) {
                // Leave the terminating CRLF
                trailer = data.mid(pos + 2);
                data.chop(data.length() - (pos + 2));
            }

            msgSize = data.length();
        } else {
            trailer = QByteArray(orgData.constData() + (bypass + remainder), orgData.length() - (bypass + remainder));
        }

        if (!flagsParsed) {
            // See if the flags follow the message data
            flagsParsed = parseFlags(QString(trailer), flags);
        }

        // Append the pop trailer
        data.append('.').append(QMailMessage::CRLF);

        createMail( data, uid, msgSize, flags );
    } else {
        emit nonexistentMessage(fetchUid);
    }

    d->reset();
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

    // The strings we're tokenizing use CRLF as the line delimiters - assume that the
    // caller considers the sequence to be atomic.
    if (c1 == QMailMessage::LineFeed)
        c1 = QMailMessage::CarriageReturn;
    start = str.indexOf( c1, *index, Qt::CaseInsensitive );
    if (start == -1)
        return QString::null;

    // Bypass the LF if necessary
    if (c1 == QMailMessage::CarriageReturn)
        start += 1;

    if (c2 == QMailMessage::LineFeed)
        c2 = QMailMessage::CarriageReturn;
    stop = str.indexOf( c2, ++start, Qt::CaseInsensitive );
    if (stop == -1)
        return QString::null;

    // Bypass the LF if necessary
    *index = stop + (c2 == QMailMessage::CarriageReturn ? 2 : 1);

    return str.mid( start, stop - start );
}

void ImapProtocol::createMail( const QByteArray& msg, QString& id, int size, uint flags )
{
    QMailMessage mail = QMailMessage::fromRfc2822( msg );

    mail.setStatus( QMailMessage::Incoming, true );
    if (flags & MFlag_Seen)
        mail.setStatus( QMailMessage::ReadElsewhere, true );
    if (flags & MFlag_Answered)
        mail.setStatus( QMailMessage::Replied, true );

    mail.setSize( size );
    mail.setServerUid( id.mid( id.indexOf( " " ) + 1, id.length() ));
    mail.setMessageType( QMailMessage::Email );

    emit messageFetched(mail);
}

