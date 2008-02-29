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
#ifndef QCOPBRIDGE_H
#define QCOPBRIGDE_H

#include <QObject>

class QCopBridgePrivate;
class QCopBridgePI;
class QCopBridgePIPrivate;
class QTcpSocket;
class QIODevice;

class QCopBridge : public QObject
{
    Q_OBJECT
public:
    QCopBridge( QObject *parent = 0 );
    ~QCopBridge();

    bool startTcp( int port = 0 );
    bool startSerial( const QString &port );

signals:
    void gotConnection();
    void lostConnection();

private slots:
    void newTcpConnection();
    void newSerialConnection();
    void startSerialConnection();
    void serialServerDied();
    void desktopMessage( const QString &message, const QByteArray &data );
    void disconnected( QCopBridgePI *pi );

private:
    void newSocket( QIODevice *socket );

    QCopBridgePrivate *d;
};

class QCopBridgePI : public QObject
{
    Q_OBJECT
public:
    QCopBridgePI( QIODevice *socket, QObject *parent = 0 );
    ~QCopBridgePI();

    enum State { WaitUser, WaitPass, Connected };

    void sendDesktopMessage( const QString &channel, const QString &message, const QByteArray &data );
    QIODevice *socket();

signals:
    void disconnected( QCopBridgePI *pi );
    void gotConnection();

private slots:
    void read();
    void socketDisconnected();
    void helperTimeout();
    void killTimerTimeout();

private:
    void process( const QByteArray &line );
    void send( const QByteArray &line, int _line );

    QCopBridgePIPrivate *d;
};

#endif
