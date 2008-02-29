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

#include "qphonesocket.h"

#include <qsocketnotifier.h>
#include <qfile.h>
#include <qtopialog.h>
#include <qtopianamespace.h>

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/un.h>

/*!
    \class QPhoneSocket
    \mainclass
    \brief The QPhoneSocket class provides socket functionality for phone call handlers

    Note: this class is provided for supporting Voice-Over-IP implementations
    that use the VoIP integration mechanism of Qtopia 4.1.  New Voice-Over-IP
    implementations should inherit from QTelephonyService instead.

    Phone call handlers manage extensions to the Qtopia phone server
    for providing additional call types (e.g. Voice-Over-IP).

    When a phone call handler starts up, it connects to the phone server
    on the Unix domain socket /tmp/qtopia-N/call-manager and registers
    itself with the "INIT" command (N is the QWS display identifier).

    This class provides a convenient method for phone call handlers
    to effect communication with the server.  When a command message
    arrives from the phone server, the received() signal is emitted with
    the command details.  The send() methods provide a convenient way
    to send a response message to the phone server.

    Phone call handlers do not need to use this class.  They can connect
    to the Unix domain socket directly with their own mechanism.  The handler
    is then responsible for all data traffic handling, parsing, etc.

    \ingroup telephony
*/

class QPhoneSocketPrivate
{
public:
    QPhoneSocketPrivate()
    {
        fd = -1;
        isServer = false;
        len = 0;
    }
    int fd;
    bool isServer;
    QByteArray line;
    int len;
};

/*!
    Create an object that manages connections to the Qtopia phone server
    for phone call handlers and attach it to \a parent.
*/
QPhoneSocket::QPhoneSocket( QObject *parent )
    : QObject( parent )
{
    d = new QPhoneSocketPrivate();
    d->fd = ::socket( AF_UNIX, SOCK_STREAM, 0 );
    if ( d->fd == -1 ) {
        perror("QPhoneSocket: socket");
        return;
    }
    struct sockaddr_un addr;
    ::memset( &addr, 0, sizeof(addr) );
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path,
            (Qtopia::tempDir()+"call-manager").toLatin1().constData(),
            sizeof( addr.sun_path ));
    if ( ::connect( d->fd, (struct sockaddr *)&addr, sizeof(addr) ) < 0 ) {
        qLog(Modem) << "QPhoneSocket: cannot connect to" << addr.sun_path;
        ::close( d->fd );
        d->fd = -1;
        return;
    }
    init();
}

/*!
    \internal
*/
QPhoneSocket::QPhoneSocket( int fd, QObject *parent )
    : QObject( parent )
{
    d = new QPhoneSocketPrivate();
    d->fd = fd;
    if ( fd == -1 ) {
        // This is the phone server asking to bind to the socket.
        d->fd = ::socket( AF_UNIX, SOCK_STREAM, 0 );
        if ( d->fd == -1 ) {
            perror("QPhoneSocket: socket");
            return;
        }
        struct sockaddr_un addr;
        ::memset( &addr, 0, sizeof(addr) );
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path,
                (Qtopia::tempDir()+"call-manager").toLatin1().constData(),
                sizeof( addr.sun_path ));
        QFile::remove( addr.sun_path );
        if ( ::bind( d->fd, (struct sockaddr *)&addr, sizeof(addr) ) < 0 ) {
            qLog(Modem) << "QPhoneSocket: cannot bind to" << addr.sun_path;
            ::close( d->fd );
            d->fd = -1;
            return;
        }
        if ( ::listen( d->fd, 5 ) < 0 ) {
            qLog(Modem) << "QPhoneSocket: cannot listen on" << addr.sun_path;
            ::close( d->fd );
            d->fd = -1;
            return;
        }
        d->isServer = true;
    }
    init();
}

void QPhoneSocket::init()
{
    // Bail out if not initialized correctly.
    if ( d->fd == -1 )
        return;

    // Turn on non-blocking mode on the socket.
    int flags = ::fcntl( d->fd, F_GETFL, 0 );
    if ( flags >= 0 )
        ::fcntl( d->fd, F_SETFL, flags | FNDELAY );

    // Create a socket notifier for reads and incoming connections.
    QSocketNotifier *notifier = new QSocketNotifier
        ( d->fd, QSocketNotifier::Read, this );
    if ( d->isServer )
        connect( notifier, SIGNAL(activated(int)), this, SLOT(readyAccept()) );
    else
        connect( notifier, SIGNAL(activated(int)), this, SLOT(readyRead()) );
}

/*!
    Destroy a phone socket handling object.
*/
QPhoneSocket::~QPhoneSocket()
{
    if ( d->fd != -1 )
        ::close( d->fd );
    delete d;
}

// Determine if an argument needs to be quoted before sending it to the peer.
static bool needsQuoting( const char *str, int len )
{
    if ( !len )
        return true;
    while ( len > 0 ) {
        int ch = (*str++ & 0xFF);
        if ( ch <= 0x20 || ch == '"' || ch == '\\' )
            return true;
        --len;
    }
    return false;
}

// Quote an argument.
static QByteArray quote( const char *str, int len )
{
    QByteArray value;
    char ch;
    value += (char)'"';
    while ( len > 0 ) {
        ch = *str++;
        --len;
        if ( ( ch & 0xFF ) < 0x20 ) {
            value += (char)'\\';
            value += (char)( ( ( ch >> 6 ) & 7 ) + '0' );
            value += (char)( ( ( ch >> 3 ) & 7 ) + '0' );
            value += (char)( ( ch & 7 ) + '0' );
        } else if ( ch == '"' || ch == '\\' ) {
            value += (char)'\\';
            value += ch;
        } else {
            value += ch;
        }
    }
    value += (char)'"';
    return value;
}

/*!
    Sends \a cmd to the peer on this connection.
*/
void QPhoneSocket::send( const QStringList& cmd )
{
    QByteArray line, arg;
    QStringList::ConstIterator iter;
    for ( iter = cmd.begin(); iter != cmd.end(); ++iter ) {
        arg = (*iter).toUtf8();
        if ( line.length() > 0 )
            line += (char)' ';
        if ( needsQuoting( arg.data(), arg.length() ) )
            line += quote( arg.data(), arg.length() );
        else
            line += arg;
    }
    qLog(Modem) << "QPhoneSocket:" << line.constData();
    line += (char)0x0D;
    line += (char)0x0A;
    if ( d->fd != -1 ) {
        const char *data = line.data();
        int len = (int)line.length();
        int temp;
        while ( len > 0 ) {
            temp = ::write( d->fd, data, len );
            if ( temp < 0 && errno != EINTR )
                break;
            else if ( temp == 0 )
                break;
            if ( temp > 0 ) {
                data += temp;
                len -= temp;
            }
        }
    }
}

/*!
    Sends \a cmd to the peer on this connection.
*/
void QPhoneSocket::send( const QString& cmd )
{
    QStringList list;
    list << cmd;
    send( list );
}

/*!
    Sends \a cmd and \a arg1 to the peer on this connection.
*/
void QPhoneSocket::send( const QString& cmd, const QString& arg1 )
{
    QStringList list;
    list << cmd;
    list << arg1;
    send( list );
}

/*!
    Sends \a cmd, \a arg1, and \a arg2 to the peer on this connection.
*/
void QPhoneSocket::send( const QString& cmd, const QString& arg1, const QString& arg2 )
{
    QStringList list;
    list << cmd;
    list << arg1;
    list << arg2;
    send( list );
}

/*!
    Sends \a cmd, \a arg1, \a arg2, and \a arg3 to the peer on this connection.
*/
void QPhoneSocket::send( const QString& cmd, const QString& arg1,
                        const QString& arg2, const QString& arg3 )
{
    QStringList list;
    list << cmd;
    list << arg1;
    list << arg2;
    list << arg3;
    send( list );
}

/*!
    \fn void QPhoneSocket::received( const QStringList& cmd )

    Signal that is emitted when \a cmd is received from the peer.
*/

/*!
    \fn void QPhoneSocket::incoming( QPhoneSocket *socket )

    Signal that is emitted when an incoming client \a socket is
    detected on a bound server port.  This is used internally
    by the Qtopia Phone Server, and will not be emitted in handler
    processes.
*/

/*!
    \fn void QPhoneSocket::closed()

    Signal that is emitted when the connection is closed by the peer.
*/

void QPhoneSocket::readyRead()
{
    // Read some more data into our internal buffer.
    const int read_size = 1024; // Must be a power of 2.
    if ( ( d->len + read_size ) > (int)(d->line.size()) ) {
        d->line.resize( ( d->len + 2 * read_size - 1 ) & ~( read_size - 1 ) );
    }
    int len;
    for(;;)
    {
        len = ::read( d->fd, d->line.data() + d->len,
                      d->line.size() - d->len );
        if ( len >= 0 )
            break;
        if ( errno != EINTR )
            return;
    }
    d->len += len;

    // Handle a connection close.
    if ( len == 0 ) {
        emit closed();
        return;
    }

    // Process lines within the incoming data.
    int posn = 0;
    const char *next;
    while ( posn < d->len ) {
        next = (const char *)::memchr
            ( d->line.data() + posn, 0x0A, d->len - posn );
        if ( !next )
            break;
        len = next - ( d->line.data() + posn );
        if ( len > 0 && *(next - 1) == 0x0D )
            processLine( d->line.data() + posn, len - 1 );
        else
            processLine( d->line.data() + posn, len );
        posn += len + 1;
    }

    // Shift the remaining data down.
    if ( posn > 0 && posn < d->len )
        ::memmove( d->line.data(), d->line.data() + posn, d->len - posn );
    d->len -= posn;
}

void QPhoneSocket::readyAccept()
{
    int fd;
    for(;;)
    {
        fd = ::accept( d->fd, (struct sockaddr *)0, 0 );
        if ( fd != -1 ) {
            emit incoming( new QPhoneSocket( fd, this ) );
        } else if ( errno != EINTR ) {
            break;
        }
    }
}

void QPhoneSocket::processLine( const char *line, int len )
{
    QStringList list;
    QByteArray arg;
    int temp;
    char ch;
    while ( len > 0 ) {
        // Extract the next argument.
        if ( *line == '"' ) {
            // Quoted argument.
            arg.resize(0);
            ++line;
            --len;
            while ( len > 0 ) {
                ch = *line++;
                --len;
                if ( ch == '"' ) {
                    break;
                } else if ( ch == '\\' ) {
                    if ( len <= 0 )
                        break;
                    ch = *line++;
                    --len;
                    if ( ch >= '0' && ch <= '7' && len >= 2 &&
                         line[0] >= '0' && line[0] <= '7' &&
                         line[1] >= '0' && line[1] <= '7' ) {
                        temp = ( ch - '0' ) * 64 +
                               ( line[0] - '0' ) * 8 +
                               ( line[1] - '0' );
                        arg += (char)temp;
                        line += 2;
                        len -= 2;
                    } else {
                        arg += ch;
                    }
                } else {
                    arg += ch;
                }
            }
            list << QString::fromUtf8( arg.data(), arg.length() );
        } else {
            // Unquoted argument.
            temp = 1;
            while ( temp < len && line[temp] != ' ' )
                ++temp;
            list << QString::fromUtf8( line, temp );
            line += temp;
            len -= temp;
        }

        // Skip white space separating the arguments.
        while ( len > 0 && *line == ' ' ) {
            ++line;
            --len;
        }
    }
    if ( list.count() > 0 ) {
        emit received( list );
    }
}
