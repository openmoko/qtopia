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

#ifndef WAVECOMMUTLIPLEXER_H
#define WAVECOMMUTLIPLEXER_H

#include <qserialiodevicemultiplexerplugin.h>
#include <qserialiodevicemultiplexer.h>

class WavecomMultiplexerPlugin : public QSerialIODeviceMultiplexerPlugin
{
    Q_OBJECT
public:
    WavecomMultiplexerPlugin( QObject* parent = 0 );
    ~WavecomMultiplexerPlugin();

    bool detect( QSerialIODevice *device );
    QSerialIODeviceMultiplexer *create( QSerialIODevice *device );
};

class WavecomCommandChannel : public QSerialIODevice
{
    Q_OBJECT
public:
    WavecomCommandChannel( QSerialIODevice *device, QObject *parent = 0 );
    ~WavecomCommandChannel();

    // Override methods from QIODevice.
    bool open( OpenMode mode );
    void close();
    qint64 bytesAvailable() const;
    bool waitForReadyRead( int msecs );

    // Override methods from QSerialIODevice.
    bool dtr() const;
    void setDtr( bool value );
    bool dsr() const;
    bool carrier() const;
    bool rts() const;
    void setRts( bool value );
    bool cts() const;
    void discard();
    void abortDial();
    QAtChat *atchat();

    void add( const char *data, uint len );

protected:
    qint64 readData( char *data, qint64 maxlen );
    qint64 writeData( const char *data, qint64 len );
    void writeInner( int lead, int type, const char *data, uint len );
    void writeBlock( int lead, int type, const char *data, uint len );

private:
    QSerialIODevice *device;
    int used;
    QByteArray buffer;
    bool needRestart;
    bool waitingForReadyRead;
};

class WavecomDataChannel : public WavecomCommandChannel
{
    Q_OBJECT
public:
    WavecomDataChannel( QSerialIODevice *device, QObject *parent = 0 );
    ~WavecomDataChannel();

    // Override methods from QSerialIODevice.
    bool dtr() const;
    void setDtr( bool value );
    bool dsr() const;
    bool carrier() const;
    bool rts() const;
    void setRts( bool value );
    bool cts() const;
    bool waitForReady() const;

    void setStatus( int status );
    void busy();
    void emitReady() { emit ready(); }

protected:
    qint64 writeData( const char *data, qint64 len );

private:
    int incomingStatus;
    int outgoingStatus;

    void updateStatus();
};

class WavecomMultiplexerPrivate;

class WavecomMultiplexer : public QSerialIODeviceMultiplexer
{
    Q_OBJECT
    friend class WavecomCommandChannel;
public:
    WavecomMultiplexer( QSerialIODevice *device, QObject *parent = 0 );
    ~WavecomMultiplexer();

    QSerialIODevice *channel( const QString& name );

private slots:
    void incoming();

private:
    WavecomMultiplexerPrivate *d;
};

#endif /* WAVECOMMUTLIPLEXER_H */
