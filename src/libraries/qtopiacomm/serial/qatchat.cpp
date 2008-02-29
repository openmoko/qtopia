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

#include <qatchat.h>
#include <qatresult.h>
#include <qserialiodevice.h>
#include <qtopiacomm/private/qprefixmatcher_p.h>
#include <qtopiacomm/private/qatchat_p.h>
#include <qtopialog.h>
#include <qtimer.h>

/*!
    \class QAtChat
    \brief The QAtChat class provides a simple mechanism for sending AT commands to modems
    \ingroup communication

    The QAtChat interface provides a simple mechanism for sending AT
    commands to modems and retrieving the responses to those commands.
    It can be preferable to manually processing the raw binary data from
    a serial device.

    QAtChat are obtained by calling QSerialIODevice::atchat().

    \sa QSerialIODevice
*/

class QAtChatCommandPrivate
{
public:
    QAtChatCommandPrivate( const QString& command, QAtResult::UserData *data )
    {
        this->command = command;
        primed = false;
        forcedAbort = false;
        result.setUserData( data );
    }

public:
    QString command;
    QString pdu;
    QAtResult result;
    bool primed;
    bool forcedAbort;
    QAtChatCommand *next;
};

class QAtChatPrivate
{
public:
    QAtChatPrivate( QSerialIODevice *_device )
    {
        fromChar = 'F';
        toChar = 'T';
        notifyChar = 'N';
        unknownChar = '?';
        device = _device;
        first = 0;
        last = 0;
        suspended = false;
        deadTimeout = -1;
        deadTimer = new QTimer();
        deadTimer->setSingleShot( true );
        requestor = 0;
        noModemReported = false;
        retryOnNonEcho = -1;
        retryTimer = new QTimer();
        retryTimer->setSingleShot( true );
    }
    ~QAtChatPrivate()
    {
        QAtChatCommand *cmd = first;
        QAtChatCommand *next;
        while ( cmd != 0 ) {
            next = cmd->d->next;
            delete cmd;
            cmd = next;
        }
        delete deadTimer;
        if ( requestor )
            delete requestor;
    }

public:
    QSerialIODevice *device;
    QAtChatCommand *first;
    QAtChatCommand *last;
    QString line;
    QString lastNotification;
    char toChar;
    char fromChar;
    char notifyChar;
    char unknownChar;
    QPrefixMatcher *matcher;
    bool suspended;
    bool noModemReported;
    int retryOnNonEcho;
    int deadTimeout;
    QTimer *deadTimer;
    QAtChatLineRequest *requestor;
    QTimer *retryTimer;
};

static QString toHex( const QByteArray& binary )
{
    QString str = "";
    static char const hexchars[] = "0123456789ABCDEF";

    for ( int i = 0; i < binary.size(); i++ ) {
        str += (QChar)(hexchars[ (binary[i] >> 4) & 0x0F ]);
        str += (QChar)(hexchars[ binary[i] & 0x0F ]);
    }

    return str;
}

static QByteArray fromHex( const QString& hex )
{
    QByteArray bytes;
    uint ch;
    int posn;
    int nibble, value, flag, size;

    flag = 0;
    value = 0;
    size = 0;
    for ( posn = 0; posn < hex.length(); ++posn ) {
        ch = (uint)( hex[posn].unicode() );
        if ( ch >= '0' && ch <= '9' ) {
            nibble = ch - '0';
        } else if ( ch >= 'A' && ch <= 'F' ) {
            nibble = ch - 'A' + 10;
        } else if ( ch >= 'a' && ch <= 'f' ) {
            nibble = ch - 'a' + 10;
        } else {
            continue;
        }
        value = (value << 4) | nibble;
        flag = !flag;
        if ( !flag ) {
            bytes.resize( size + 1 );
            bytes[size++] = (char)value;
            value = 0;
        }
    }

    return bytes;
}

QAtChatCommand::QAtChatCommand( const QString& command, QAtResult::UserData *data )
{
    d = new QAtChatCommandPrivate( command, data );
}

QAtChatCommand::QAtChatCommand( const QString& command, const QByteArray& pdu,
                                QAtResult::UserData *data )
{
    d = new QAtChatCommandPrivate( command, data );
    d->pdu = toHex( pdu ) + "\032";
}

QAtChatCommand::~QAtChatCommand()
{
    delete d;
}

void QAtChatCommand::emitDone()
{
    emit done( d->result.ok() && !(d->forcedAbort), d->result );
}

QAtChat::QAtChat( QSerialIODevice *device )
    : QObject( device )
{
    d = new QAtChatPrivate( device );
    connect( d->deadTimer, SIGNAL(timeout()), this, SLOT(timeout()) );
    connect( d->retryTimer, SIGNAL(timeout()), this, SLOT(retryTimeout()) );

    // Initialize the matcher, which we use to determine what needs
    // to be done with incoming lines.
    d->matcher = new QPrefixMatcher( this );
    d->matcher->add( "OK", QPrefixMatcher::OKTerminator );
    d->matcher->add( "+CME ERROR:", QPrefixMatcher::Terminator );
    d->matcher->add( "+CMS ERROR:", QPrefixMatcher::Terminator );
    d->matcher->add( "+EXT ERROR:", QPrefixMatcher::Terminator );
    d->matcher->add( "+CMT:", QPrefixMatcher::NotificationWithPDU );
    d->matcher->add( "+CDS:", QPrefixMatcher::NotificationWithPDU );
    d->matcher->add( "+CBM:", QPrefixMatcher::NotificationWithPDU );
    d->matcher->add( "ERROR", QPrefixMatcher::Terminator );
    d->matcher->add( "CONNECT", QPrefixMatcher::TerminatorOrNotification );
    d->matcher->add( "NO CARRIER", QPrefixMatcher::TerminatorOrNotification );
    d->matcher->add( "NO ANSWER", QPrefixMatcher::TerminatorOrNotification );
    d->matcher->add( "BUSY", QPrefixMatcher::TerminatorOrNotification );
    d->matcher->add( "NO DIALTONE", QPrefixMatcher::TerminatorOrNotification );
    d->matcher->add( "AT", QPrefixMatcher::CommandEcho );

    // Work arounds for Wavecom modems.  (a) Error 515 is actually an
    // intermediate warning that can be just ignored.  (b) AT+CPIN? does
    // not always give "OK" - it sometimes stops at "+CPIN: value".
    d->matcher->add( "+CME ERROR: 515", QPrefixMatcher::Terminator ); // XXX

    // Ask the device to tell us when it is ready to read.
    connect( device, SIGNAL(readyRead()), this, SLOT(incoming()) );

    // Make sure that the device is open, ready to receive AT commands.
    if ( !device->isOpen() )
        device->open( QIODevice::ReadWrite );
}

QAtChat::~QAtChat()
{
    delete d;
}

/*!
    Send \a command to the underlying device.  If the command fails,
    the caller will not be notified.
*/
void QAtChat::chat( const QString& command )
{
    QAtChatCommand *cmd = new QAtChatCommand( command, 0 );
    queue( cmd );
}

/*!
    Send \a command to the underlying device.  When the command finishes,
    notify \a slot on \a target.  The slot has the signature
    \c{done(bool,QAtResult)}.  The boolean parameter indicates if
    the command succeeded or not, and the QAtResult parameter contains
    the full result data.

    The optional \a data parameter can be used to pass extra user data
    that will be made available to the target slot in the QAtResult::userData()
    field.
*/
void QAtChat::chat( const QString& command, QObject *target, const char *slot,
                    QAtResult::UserData *data )
{
    QAtChatCommand *cmd = new QAtChatCommand( command, data );
    connect( cmd, SIGNAL(done(bool,QAtResult)), target, slot );
    queue( cmd );
}

/*!
    Send \a command to the underlying device, followed by \a pdu
    on the next line.  When the command finishes, notify \a slot on \a target.

    The \a pdu will be transmitted in hexadecimal, followed by a CTRL-Z.

    The optional \a data parameter can be used to pass extra user data
    that will be made available to the target slot in the QAtResult::userData()
    field.

    \sa chat()
*/
void QAtChat::chatPDU( const QString& command, const QByteArray& pdu,
                       QObject *target, const char *slot,
                       QAtResult::UserData *data )
{
    QAtChatCommand *cmd = new QAtChatCommand( command, pdu, data );
    if ( target && slot )
        connect( cmd, SIGNAL(done(bool,QAtResult)), target, slot );
    queue( cmd );
}

/*!
    Register \a type as an unsolicited notification on this object.
    Whenever a line is received from the modem that starts with
    \a type, the indicated \a slot on \a target will be called.
    The slot has the signature \c{notification(const QString&)}.
    The entire notification, including the \a type prefix,
    will be passed to the slot.

    If \a mayBeCommand is true, then the notification type may sometimes
    appear as a command result.  When it does, the command's completion
    slot should be called, not the unsolicited notification slot.
    An example is \c{+CREG:}, which can appear in response to an
    \c{AT+CREG} command, or as an unsolicited notification.
*/
void QAtChat::registerNotificationType
        ( const QString& type, QObject *target,
          const char *slot, bool mayBeCommand )
{
    d->matcher->add( type, QPrefixMatcher::Notification, mayBeCommand,
                     target, slot );
}

/*!
    Abort an \c{ATD} command.  Usually just sends and empty line to
    the modem, but some modems use need \c{ATH} instead.
*/
void QAtChat::abortDial()
{
    if ( d->first && d->first->d->command.startsWith( "ATD" ) ) {
        d->device->abortDial();
        d->first->d->forcedAbort = true;
    }
}

/*!
    Suspend the AT chat process from the underlying device so
    that read() and write() can be used to process binary data.
    Call resume() to restart the AT chat process after the binary data.

    \sa resume()
*/
void QAtChat::suspend()
{
    if ( !d->suspended ) {
        d->suspended = true;

        // Disconnect the incoming() signal, so that we don't accidentally
        // steal the incoming data from the other entity that wants it.
        disconnect( d->device, SIGNAL(readyRead()), this, SLOT(incoming()) );
    }
}

/*!
    Resume the AT chat process after a suspension.  Any data that
    is currently in the buffer will be processed for unsolicited
    notifications.

    \sa suspend()
*/
void QAtChat::resume()
{
    if ( d->suspended ) {
        d->suspended = false;

        // Force the device to be re-opened if the binary process closed it.
        if ( !d->device->isOpen() )
            d->device->open( QIODevice::ReadWrite );

        // Reconnect the incoming() signal and process any available data.
        connect( d->device, SIGNAL(readyRead()), this, SLOT(incoming()) );
        if ( d->device->bytesAvailable() > 0 ) {
            incoming();
        }
    }
}

/*!
    Set the characters to use in debug output to \a from, \a to, \a notify,
    and \a unknown.  The defaults are \c{F}, \c{T}, \c{N}, and \c{?}.
    The caller may wish to use different sets on separate channels so that
    it is clear from the debug output which channel is being used.
*/
void QAtChat::setDebugChars( char from, char to, char notify, char unknown )
{
    d->fromChar = from;
    d->toChar = to;
    d->notifyChar = notify;
    d->unknownChar = unknown;
}

/*!
    Get the current link dead detection timeout in milliseconds.
    If this object sends a command to the link and it does not
    receive a response within the specified time, it will emit
    the dead() signal.  If the value is -1 (the default), the
    link dead detection timeout will be disabled.

    When the link dead detection timeout expires, all pending
    commands will fail with QAtResult::Dead as the result code.

    \sa setDeadTimeout(), dead()
*/
int QAtChat::deadTimeout() const
{
    return d->deadTimeout;
}

/*!
    Set the link dead detection timeout to \a msec milliseconds.
    If this object sends a command to the link and it does not
    receive a response within the specified time, it will emit
    the dead() signal.  If \a msec is -1, the link dead detection
    timeout will be disabled.

    When the link dead detection timeout expires, all pending
    commands will fail with QAtResult::Dead as the result code.

    \sa deadTimeout(), dead()
*/
void QAtChat::setDeadTimeout( int msec )
{
    d->deadTimeout = msec;
    if ( d->deadTimer->isActive() ) {
        if ( msec != -1 ) {
            d->deadTimer->start( msec );
        } else {
            d->deadTimer->stop();
        }
    }
}

/*!
    Get the retry on non-echo timeout.  The default is -1.

    \sa setRetryOnNonEcho()
*/
int QAtChat::retryOnNonEcho() const
{
    return d->retryOnNonEcho;
}

/*!
    Set the retry on non-echo timeout to \a msec.  If \a msec is not -1,
    then if an AT command is not echoed by the modem within the
    time period, the command will be automatically retried.  This is
    sometimes necessary to restart chat operations after a modem stall.

    \sa retryOnNonEcho()
*/
void QAtChat::setRetryOnNonEcho( int msec )
{
    d->retryOnNonEcho = msec;
}

/*!
    Set \c{+CPIN} as a terminator for the \c{AT+CPIN?} command.
    Needed on some modems that do not send \c{OK}.
*/
void QAtChat::setCPINTerminator()
{
    d->matcher->add( "+CPIN:", QPrefixMatcher::FuzzyTerminator );
}

/*!
    Request that the next line from the modem be delivered to
    \a slot on \a target.  This is used to collect up extra
    lines of data on an unsoliticited response.  The slot
    takes a single QString parameter.
*/
void QAtChat::requestNextLine( QObject *target, const char *slot )
{
    if ( d->requestor )
        delete d->requestor;
    d->requestor = new QAtChatLineRequest( target, slot );
}

/*!
    Send \a command directly to the modem without waiting for a response,
    and without waiting for any existing commands to complete.  This is
    typically used to send \c{ATH} to a modem to abort an \c{ATD} command.
*/
void QAtChat::send( const QString& command )
{
    writeLine( command );
}

/*!
    \fn void QAtChat::pduNotification( const QString& type, const QByteArray& pdu )

    Signal that is emitted when a PDU notification such as \c{+CMT},
    \c{+CDS}, or \c{+CBM} arrives.  The \a type parameter contains the
    type and PDU length (e.g. \c{+CBM: 88}), and the \a pdu parameter
    contains the binary data for the PDU.
*/

/*!
    \fn void QAtChat::callNotification( const QString& type )

    Signal that is emitted when a call-related notification such as
    \c{CONNECT}, \c{NO CARRIER}, \c{BUSY}, etc occurs, but which was
    not associated with a corresponding \c{ATD} command.  The \a type
    parameter is the text of the notification.
*/

/*!
    \fn void QAtChat::dead()

    Signal that is emitted when the link is detected to be dead.

    \sa deadTimeout(), setDeadTimeout()
*/

void QAtChat::queue( QAtChatCommand *command )
{
    // Make sure that the chat object is active on the device.
    resume();

    // Add the command to the queue.
    command->d->next = 0;
    if ( d->last )
        d->last->d->next = command;
    else
        d->first = command;
    d->last = command;

    // If this is the first command on the queue, then
    // transmit it to the modem device.
    if ( d->first == command ) {
        prime();
    }
}

void QAtChat::done()
{
    QAtChatCommand *cmd;

    // Remove the first command from the queue.
    cmd = d->first;
    d->first = cmd->d->next;
    if ( !( d->first ) )
        d->last = 0;

    // Stop the dead timer.
    d->deadTimer->stop();

    // Stop the retry on non-echo timer.
    d->retryTimer->stop();

    // Emit the "done" signal for the command.
    cmd->emitDone();

    // Destroy the command object.
    delete cmd;
}

bool QAtChat::writeLine( const QString& line )
{
    bool result;
    if ( ! d->device->isValid() ) {
        if ( !d->noModemReported ) {
            qWarning() << "**********************************************************";
            qWarning() << "* WARNING: Failed to open modem device";
            qWarning() << "* Modem functions will be disabled";
            qWarning() << "**********************************************************";
            d->noModemReported = true;
        }
        result = false;
    } else {
        QString nline = line + "\r";
        d->device->write( nline.toLatin1() );
        result = true;
    }
    qLog(AtChat) << d->toChar << ":" << line;
    return result;
}

void QAtChat::writePduLine( const QString& line )
{
    if ( ! d->device->isValid() )
        return;
    d->device->write( line.toLatin1() );
    qLog(AtChat) << d->toChar << ":" << line;
}

bool QAtChat::processLine( const QString& line )
{
    // Ignore blank lines.
    if ( line.isEmpty() )
        return false;

    // Determine what to do with the line.
    QString command;
    if ( d->first )
        command = d->first->d->command;
    switch ( d->matcher->lookup( line, command ) ) {

        case QPrefixMatcher::Unknown:
        {
            // Are we looking for the rest of +CMT/+CDS/+CBM?
            if ( ! d->lastNotification.isEmpty() ) {
                qLog(AtChat) << d->notifyChar << ":" << line;
                QString type = d->lastNotification;
                QByteArray pdu;
                if ( type.count( QChar(',') ) >= 2 )
                    pdu = line.toLatin1();      // Data sent in text mode.
                else
                    pdu = fromHex( line );      // Data sent in PDU mode.
                d->lastNotification = QString();
                emit pduNotification( type, pdu );
                return false;
            }

            // If we are looking for a request line, then send it on.
            if ( d->requestor ) {
                qLog(AtChat) << d->notifyChar << ":" << line;
                QAtChatLineRequest *requestor = d->requestor;
                d->requestor = 0;
                requestor->sendLine( line );
                delete requestor;
                break;
            }

            // If we are processing a command, then add this line to
            // the contents of the command's result.  Otherwise ignore it.
            if ( d->first ) {
                qLog(AtChat) << d->fromChar << ":" << line;
                d->first->d->result.append( line );
                return true;
            } else {
                qLog(AtChat) << d->unknownChar << ":" << line;
            }
        }
        break;

        case QPrefixMatcher::OKTerminator:
        case QPrefixMatcher::Terminator:
        {
            // Terminator for the current command (e.g. OK or ERROR).
            if ( d->first ) {
                qLog(AtChat) << d->fromChar << ":" << line;
                d->first->d->result.setResult( line );
                done();
                primeIfNecessary();
                return true;
            } else {
                qLog(AtChat) << d->unknownChar << ":" << line;
            }
        }
        break;

        case QPrefixMatcher::FuzzyTerminator:
        {
            // If we are processing a command, then terminate it as though
            // we had received an "OK" terminator.
            if ( d->first ) {
                qLog(AtChat) << d->fromChar << ":" << line;
                d->first->d->result.append( line );
                d->first->d->result.setResult( "OK" );
                done();
                primeIfNecessary();
                return true;
            } else {
                qLog(AtChat) << d->unknownChar << ":" << line;
            }
        }
        break;

        case QPrefixMatcher::TerminatorOrNotification:
        {
            // If the command is ATD, ATA, ATH, or AT+CHLD, then this
            // is a terminator.  Otherwise it is an unsolicited notification.
            if ( d->first &&
                 ( d->first->d->command.startsWith( "ATD" ) ||
                   d->first->d->command.startsWith( "ATA" ) ||
                   d->first->d->command.startsWith( "ATH" ) ||
                   d->first->d->command.startsWith( "AT+CHLD=" ) ) ) {
                qLog(AtChat) << d->fromChar << ":" << line;
                d->first->d->result.setResult( line );
                done();
                primeIfNecessary();
                return true;
            } else {
                qLog(AtChat) << d->notifyChar << ":" << line;
                emit callNotification( line );
            }
        }
        break;

        case QPrefixMatcher::CommandEcho:
        {
            // We got a command echo, so no need to retry the command.
            d->retryTimer->stop();
        }
        // Fall through to the next case.

        case QPrefixMatcher::Ignore:
        {
            // Log the line, but just ignore it for now.
            qLog(AtChat) << d->fromChar << ":" << line;

            // Some Ericsson modems have a bug that cause them to splat
            // PDU notifications (e.g. +CBM) into the middle of a command
            // echo instead of on a line of its own.  This can cause QAtChat
            // to get confused and treat a command response line as the
            // notification PDU.  The following line clears the notification
            // state to recover from this error condition.
            d->lastNotification = QString();
        }
        break;

        case QPrefixMatcher::NotificationWithPDU:
        {
            // Notification with a PDU on the next line (+CMT/+CDS/+CBM).
            if ( line.length() > 9 && !line.contains( QChar(',') ) ) {
                // The PDU is on the same line.  Technically an error,
                // but does happen on some Ericsson phones.  We assume
                // that the length of the PDU is two characters long.
                QString type = line.left(8);
                QByteArray pdu = fromHex( line.mid(8) );
                emit pduNotification( type, pdu );
            } else {
                // Wait for the next line to arrive before continuing.
                d->lastNotification = line;
            }
        }
        // Fall through to the next case.

        case QPrefixMatcher::Notification:
        {
            // Recognised as a notification.  The prefix matcher
            // has already dispatched the notification for us.
            qLog(AtChat) << d->notifyChar << ":" << line;
        }
        break;
    }

    return false;
}

void QAtChat::incoming()
{
    char buf[256];
    char ch;
    int posn;
    bool resetTimer = false;

    // Process the lines in the incoming input.
    posn = 0;
    while ( d->device->getChar(&ch) ) {
        if ( ch == 0x0A ) {
            // LF terminates the line.
            buf[posn] = '\0';
            d->line += buf;
            resetTimer |= processLine( d->line );
            d->line = "";
            posn = 0;
        } else if ( ch != 0x00 && ch != 0x0D ) {
            buf[posn++] = (char)ch;
            if ( posn >= (int)(sizeof(buf) - 1) ) {
                buf[posn] = '\0';
                d->line += buf;
                posn = 0;
            }
        }
    }
    if ( posn > 0 ) {
        buf[posn] = '\0';
        d->line += buf;
    }

    // Reset the dead timer if we got some useful data.
    if ( resetTimer && d->deadTimer->isActive() ) {
        d->deadTimer->start( d->deadTimeout );
    }

    // If the line buffer contains "> ", then we need to send the pdu.
    if ( d->line == "> " && d->first ) {
        d->line = "";

        // Write the PDU, terminated by ^Z, but without a trailing CR.
        // A trailing CR will cause Wavecom modems to fail when they
        // are in multiplexing mode.
        writePduLine( d->first->d->pdu );
    }
}

void QAtChat::timeout()
{
    // Fail all of the commands that are still on the queue.
    while ( d->first ) {
        d->first->d->result.setResultCode( QAtResult::Dead );
        done();
    }

    // Emit the dead() signal to let interested parties know the link is dead.
    emit dead();
}

void QAtChat::failCommand()
{
    if ( !d->device->isValid() && d->first ) {
        qLog(AtChat) << d->toChar << ":"
                     << "ERROR - command channel is not active";
        d->first->d->result.setResultCode( QAtResult::Error );
        done();
        primeIfNecessary();
    }
}

void QAtChat::retryTimeout()
{
    // Prime the command into the channel again.
    prime();
}

void QAtChat::prime()
{
    QAtChatCommand *cmd = d->first;

    // Bail out if there are no commands to be sent.
    if ( !cmd ) {
        return;
    }

    // Write the command to the serial output stream.
    if ( !writeLine( cmd->d->command ) ) {
        // The command stream is not valid.  Fail the command.
        QTimer::singleShot( 0, this, SLOT(failCommand()) );
    }

    // Mark the command as primed.
    cmd->d->primed = true;

    // Reset the dead timer.
    if ( d->deadTimeout != -1 ) {
        d->deadTimer->start( d->deadTimeout );
    }

    // Reset the retry on non-echo timer, to detect the command echo.
    if ( d->retryOnNonEcho != -1 ) {
        d->retryTimer->start( d->retryOnNonEcho );
    }
}

void QAtChat::primeIfNecessary()
{
    if ( d->first && !(d->first->d->primed) ) {
        prime();
    }
}
