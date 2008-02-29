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
#include "qcopbridge.h"
#include "syncauthentication.h"
#include "serialport.h"
#include "log.h"
#include "qdlinkhelper.h"
#include "qcopchannel_qd.h"

#include <custom-qtopia.h>

#if defined(Q_WS_X11)
#include <qcopchannel_x11.h>
#else
#include <QCopChannel>
#endif
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimerEvent>
#include <QtopiaApplication>
#include <QTextBrowser>
#include <version.h>
#include <QTimer>

#ifndef QDSYNC_MODEL
#define QDSYNC_MODEL "Unspecified Device"
#endif

#ifndef QDSYNC_SYSTEM
#define QDSYNC_SYSTEM "Qtopia"
#endif

extern bool qdsync_send_200;

class QCopBridgePrivate
{
public:
    QTcpServer *tcpServer;
    SerialPort *serialServer;
    QString serialPort;
    QList<QCopBridgePI*> connections;
};

// ====================================================================

QCopBridge::QCopBridge( QObject *parent )
    : QObject( parent )
{
    qdsync_send_200 = true;

    d = new QCopBridgePrivate;
    d->tcpServer = new QTcpServer( this );
    connect( d->tcpServer, SIGNAL(newConnection()), this, SLOT(newTcpConnection()) );

    d->serialServer = 0;

    qdsync::QCopChannel *bridgeChannel = new qdsync::QCopChannel( "QD/*", this );
    connect( bridgeChannel, SIGNAL(received(QString,QByteArray)),
             this, SLOT(desktopMessage(QString,QByteArray)) );
}

QCopBridge::~QCopBridge()
{
    TRACE(QDSync) << "QCopBridge::~QCopBridge";
    foreach ( QCopBridgePI *pi, d->connections ) {
        delete pi;
    }

    // delete the serial server if it hasn't been moved to another thread
    // i.e. we haven't gotten a serial connection yet
    if (d->serialServer) {
        if (d->serialServer->thread() == thread()) {
            delete d->serialServer;
            d->serialServer = 0;
        }
    }

    delete d;
}

bool QCopBridge::startTcp( int port )
{
    return d->tcpServer->listen( QHostAddress::Any, port );
}

bool QCopBridge::startSerial( const QString &port )
{
    d->serialPort = port;
    startSerialConnection();
    return ( d->serialServer != 0 );
}

void QCopBridge::serialServerDied()
{
    TRACE(QDSync) << "QCopBridge::serialServerDied";
    d->serialServer = 0;
    QTimer::singleShot( 5000, this, SLOT(startSerialConnection()) );
}

void QCopBridge::startSerialConnection()
{
    TRACE(QDSync) << "QCopBridge::startSerialConnection";
    //d->serialServer = 0;
    if ( d->serialPort.isEmpty() )
        return;
    d->serialServer = new SerialPort( d->serialPort );
    connect( d->serialServer, SIGNAL(newConnection()), this, SLOT(newSerialConnection()) );
    if ( d->serialServer->open( QIODevice::ReadWrite ) ) {
        connect( d->serialServer, SIGNAL(destroyed()), this, SLOT(serialServerDied()) );
        USERLOG(QString("QCopBridge started on serial port %1").arg(d->serialPort));
    } else {
        delete d->serialServer;
        d->serialServer = 0;
    }
}

void QCopBridge::newTcpConnection()
{
    TRACE(QDSync) << "QCopBridge::newTcpConnection";
    while ( d->tcpServer->hasPendingConnections() ) {
        QTcpSocket *socket = d->tcpServer->nextPendingConnection();
        Q_ASSERT(socket);
        USERLOG("Got QCop Connection (TCP)");
        socket->setParent( 0 );
        newSocket( socket );
    }
}

void QCopBridge::newSocket( QIODevice *socket )
{
    QCopBridgePI *pi = new QCopBridgePI( socket, this );
    connect( pi, SIGNAL(disconnected(QCopBridgePI*)), this, SLOT(disconnected(QCopBridgePI*)) );
    connect( pi, SIGNAL(gotConnection()), this, SIGNAL(gotConnection()) );
    d->connections << pi;

    // Don't let the device sleep when it's connected (redundant?)
#if QTOPIA_VERSION >= 0x040200
#else
    QtopiaApplication::setTemporaryScreenSaverMode( QtopiaApplication::DisableSuspend );
#endif
}

void QCopBridge::newSerialConnection()
{
    TRACE(QDSync) << "QCopBridge::newSerialConnection";
    if ( d->serialServer ) {
        USERLOG("Got QCop Connection (Serial)");
        newSocket( d->serialServer );
    }
}

void QCopBridge::disconnected( QCopBridgePI *pi )
{
    TRACE(QDSync) << "QCopBridge::disconnected";
    d->connections.removeAll( pi );
    QIODevice *socket = pi->socket();
    if ( socket == d->serialServer ) {
        USERLOG("Lost QCop Connection (Serial)");
    } else {
        USERLOG("Lost QCop Connection (TCP)");
    }
    socket->deleteLater();
    delete pi;

    // Let the device sleep again when no more connections exist
#if QTOPIA_VERSION >= 0x040200
#else
    if ( d->connections.count() == 0 )
        QtopiaApplication::setTemporaryScreenSaverMode( QtopiaApplication::Enable );
#endif

    if ( d->connections.count() == 0 )
        emit lostConnection();
}

void QCopBridge::desktopMessage( const QString &message, const QByteArray &data )
{
    if ( message == "forwardedMessage(QString,QString,QByteArray)" ) {
        TRACE(QDSync) << "QCopBridge::desktopMessage";
        // Don't do all of this work if the LOG() statement isn't going to do anything
        if ( QDSync_TraceLog::enabled() ) {
            QDataStream stream( data );
            QString channel;
            QString message;
            QByteArray data;
            stream >> channel >> message >> data;
            LOG() << "Forwarding message" << channel << message << data << "to Qtopia Sync Agent";
        }
        // send the command to all open connections
        foreach ( QCopBridgePI *pi, d->connections )
            pi->sendDesktopMessage( "QD/QDSync", message, data );
    }
}

// ====================================================================

#define _send(x) send(x,__LINE__)

class QCopBridgePIPrivate
{
public:
    QDLinkHelper *helper;
    QIODevice *socket;
    enum QCopBridgePI::State state;
    QTimer *killTimer;
};

// ====================================================================

QCopBridgePI::QCopBridgePI( QIODevice *socket, QObject *parent )
    : QObject( parent )
{
    TRACE(QDSync) << "QCopBridgePI::QCopBridgePI";
    d = new QCopBridgePIPrivate;
    d->helper = new QDLinkHelper( socket, this );
    connect( d->helper, SIGNAL(timeout()), this, SLOT(helperTimeout()) );
    d->socket = d->helper->socket();
    d->state = QCopBridgePI::WaitUser;
    d->killTimer = new QTimer( this );
    d->killTimer->setSingleShot( true );
    d->killTimer->setInterval( 5000 ); // You have 5 seconds to respond to the 220 message
    connect( d->killTimer, SIGNAL(timeout()), this, SLOT(killTimerTimeout()) );

    connect( d->socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()) );
    connect( d->socket, SIGNAL(readyRead()), this, SLOT(read()) );

    _send( QString("220 Qtopia %1;challenge=%2;"
                                 "loginname=%3;"
                                 "displayname=%4;"
                                 "protocol=2;"
                                 "system=%5;"
                                 "model=%6;"
                                 "hexversion=%7")
            .arg(QTOPIA_VERSION_STR)
            .arg(QString(SyncAuthentication::serverId()))
            .arg(QString(SyncAuthentication::loginName()))
            .arg(QString(SyncAuthentication::ownerName()))
            .arg(QString(QDSYNC_SYSTEM))
            .arg(QString(QDSYNC_MODEL))
            .arg(QTOPIA_VERSION)
            .toLocal8Bit() );

    LOG() << "starting kill timer (5 seconds)";
    d->killTimer->start();
}

QCopBridgePI::~QCopBridgePI()
{
    delete d->helper;
    delete d;
}

QIODevice *QCopBridgePI::socket()
{
    Q_ASSERT(d->helper);
    return d->helper->rawSocket();
}

void QCopBridgePI::read()
{
    TRACE(QDSync) << "QCopBridge::read";
    while ( d->socket && d->socket->canReadLine() ) {
        if ( d->killTimer->isActive() ) {
            LOG() << "stopping kill timer";
            d->killTimer->stop();
        }
        QByteArray line = d->socket->readLine();
        process(line.trimmed());
    }
}

void QCopBridgePI::send( const QByteArray &line, int _line )
{
    TRACE(QDSync) << "QCopBridge::send";
    if ( _line != -1 ) LOG() << line << "from line" << _line;
    if ( !d->socket ) return;
    QTextStream stream( d->socket );
    stream << line << endl;
}

void QCopBridgePI::process( const QByteArray &line )
{
    QList<QByteArray> msg = line.split(' ');

    // command token
    QByteArray cmd = msg[0].toUpper();

    TRACE(QDSync) << "QCopBridgePI::process" << line;

    // argument token
    QByteArray arg;
    if ( msg.count() >= 2 )
	arg = msg[1];

    // we always respond to QUIT, regardless of state
    if ( cmd == "QUIT" ) {
	_send( "211 Have a nice day!" );
	d->socket->close();
	return;
    }

    // waiting for user name
    if ( d->state == QCopBridgePI::WaitUser ) {
	if ( cmd != "USER" || msg.count() < 2 || !SyncAuthentication::checkUser( arg ) ) {
            _send( "530 Please login with USER and PASS" );
	    return;
	}
	_send( "331 User name ok, need password" );
	d->state = QCopBridgePI::WaitPass;
	return;
    }

    // waiting for password
    if ( d->state == QCopBridgePI::WaitPass ) {
	if ( cmd != "PASS" || !SyncAuthentication::checkPassword( arg ) ) {
	    _send( "530 Please login with USER and PASS" );
	    return;
	}
	_send( "230 User logged in, proceed" );
	d->state = QCopBridgePI::Connected;
        emit gotConnection();
	return;
    }

    // call (CALL/CALLB)
    if ( cmd == "CALL" || cmd == "CALLB" ) {
	// example: call QPE/System execute(QString) addressbook
	// example: callb QPE/System execute(QString) base64datbase64data
	if ( msg.count() < 3 ) {
	    _send( "500 Syntax error, command unrecognized" );
            return;
	}

        QByteArray channel = msg[1];
        QByteArray message = msg[2].trimmed();
        QByteArray data;
        if ( cmd == "CALLB" ) {
            if ( msg.count() >= 4 )
                data = QByteArray::fromBase64( msg[3].trimmed() );
        } else {
            int paramIndex = message.indexOf('(');
            if ( paramIndex == -1 || message.indexOf(')') != message.length() - 1 ) {
                _send( "500 Syntax error, command unrecognized" );
                return;
            }
            QByteArray params = message.mid(paramIndex+1, message.length() - paramIndex - 2);

            QDataStream stream( &data, QIODevice::WriteOnly );

            int msgId = 3;

            QList<QByteArray> paramList;
            if ( !params.isEmpty() )
                paramList = params.split(',');
            if ( paramList.count() > msg.count() - 3 ) {
                _send( "500 Syntax error, command unrecognized" );
                return;
            }

            foreach ( const QByteArray &param, paramList ) {
                QByteArray arg = msg[msgId++];
                arg.replace( "&0x20;", " " );
                arg.replace( "&amp;", "&" );
                arg.replace( "&0x0d;", "\n" );
                arg.replace( "&0x0a;", "\r" );
                if ( param == "QString" ) {
                    stream << QString(arg);
                } else if ( param == "int" || param == "bool" ) {
                    stream << arg.toInt();
                } else {
                    _send( "500 Syntax error, command unrecognized" );
                    return;
                }
            }
        }

        // QPE/QDSync forwardedMessage(QString,QString,QByteArray) is a local message sent
        // from Qtopia Sync Agent. It's unpacked and delivered to local listeners only.
        if ( channel == "QPE/QDSync" && message == "forwardedMessage(QString,QString,QByteArray)" ) {
            QDataStream stream( data );
            QString _channel;
            QString _message;
            QByteArray _data;
            stream >> _channel >> _message >> _data;

            if ( !qdsync::QCopChannel::isRegistered(_channel) ) {
                _send( QString("599 ChannelNotRegistered %1").arg(_channel).toLocal8Bit() );
                return;
            }
            LOG() << "Sending LOCAL QCop:" << "channel" << _channel << "message" << _message << "data" << _data;
            qdsync::QCopChannel::send( _channel, _message, _data );
        } else {
            if ( qdsync::QCopChannel::isRegistered(channel) ) {
                // First we check if a local-only channel exists.
                LOG() << "Sending LOCAL QCop:" << "channel" << channel << "message" << message << "data" << data;
                qdsync::QCopChannel::send( channel, message, data );
            } else if ( channel.indexOf("QPE/Application/") == 0 || ::QCopChannel::isRegistered(channel) ) {
                // Messages can be sent to QPE/Application/* or any registered channel.
                // Note that this is a security hole and the default permissions most likely
                // do not allow these messages to be delivered, causing the app to be terminated.
                // If you need to allow a message through add it to the qdsync SXE profile.
                LOG() << "Sending REMOTE QCop:" << "channel" << channel << "message" << message << "data" << data;
                ::QCopChannel::send( channel, message, data );
            } else {
                _send( QString("599 ChannelNotRegistered %1").arg(QString(channel)).toLocal8Bit() );
            }
        }

        if ( qdsync_send_200 )
            _send( "200 Command okay" );
        return;
    }

    _send( "502 Command not implemented" );
}

void QCopBridgePI::sendDesktopMessage( const QString &channel, const QString &msg, const QByteArray &data )
{
    TRACE(QDSync) << "QCopBridge::sendDesktopMessage" << channel << msg << data;
    QTextStream stream( d->socket );
    stream << "CALLB " << channel << " " << msg << " "
           << data.toBase64() << endl;
}

void QCopBridgePI::socketDisconnected()
{
    TRACE(QDSync) << "QCopBridgePI::socketDisconnected";
    // This could trigger some code called from the socket!
    SyncAuthentication::clearDialogs();
    if ( d->socket ) {
        d->socket = 0;
        emit disconnected( this );
    }
}

void QCopBridgePI::helperTimeout()
{
    TRACE(QDSync) << "QCopBridgePI::helperTimeout";
    socketDisconnected();
}

void QCopBridgePI::killTimerTimeout()
{
    TRACE(QDSync) << "QCopBridgePI::killTimerTimeout";
    socketDisconnected();
}

