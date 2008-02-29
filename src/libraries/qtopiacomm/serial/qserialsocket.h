/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef QSERIALSOCKET_H
#define QSERIALSOCKET_H

#include <qserialiodevice.h>

class QTcpSocket;
class QTcpServer;

class QSerialSocketPrivate;

class QTOPIACOMM_EXPORT QSerialSocket : public QSerialIODevice
{
    Q_OBJECT
    friend class QSerialSocketServer;
private:
    QSerialSocket( QTcpSocket *socket );
public:
    QSerialSocket( const QString& host, quint16 port, QObject *parent = 0 );
    ~QSerialSocket();

    bool open( OpenMode mode );
    void close();
    bool waitForReadyRead(int msecs);
    qint64 bytesAvailable() const;

    bool dtr() const;
    void setDtr( bool value );
    bool dsr() const;
    bool carrier() const;
    bool setCarrier( bool value );
    bool rts() const;
    void setRts( bool value );
    bool cts() const;
    void discard();
    bool isValid() const;

signals:
    void closed();

protected:
    qint64 readData( char *data, qint64 maxlen );
    qint64 writeData( const char *data, qint64 len );

private slots:
    void socketReadyRead();
    void socketClosed();

private:
    QSerialSocketPrivate *d;

    void init();

    void sendModemSignal( int ch );
    void sendCommand( const char *buf, int len );

    void sendDo( int option );
    void sendDont( int option );
    void sendWill( int option );
    void sendWont( int option );
    void sendSubOption( int option, const char *buf, int len );

    void initTelnet();

    void receiveModemSignal( int ch );

    void receiveDo( int option );
    void receiveDont( int option );
    void receiveWill( int option );
    void receiveWont( int option );
    void receiveSubOption( int option, const char *buf, int len );
};

class QTOPIACOMM_EXPORT QSerialSocketServer : public QObject
{
    Q_OBJECT
public:
    explicit QSerialSocketServer( quint16 port, bool localHostOnly = true,
                                  QObject *parent = 0 );
    ~QSerialSocketServer();

    bool isListening() const;
    quint16 port() const;

signals:
    void incoming( QSerialSocket *socket );

private slots:
    void newConnection();

private:
    QTcpServer *server;
};


#endif // QSERIALSOCKET_H
