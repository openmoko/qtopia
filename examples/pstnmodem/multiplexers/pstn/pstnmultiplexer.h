/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef PSTNMULTIPLEXER_H
#define PSTNMULTIPLEXER_H

#include <qserialiodevicemultiplexerplugin.h>
#include <qserialiodevicemultiplexer.h>

class PstnMultiplexerPrivate;

class PstnMultiplexerPlugin : public QSerialIODeviceMultiplexerPlugin
{
    Q_OBJECT
public:
    PstnMultiplexerPlugin( QObject* parent = 0 );
    ~PstnMultiplexerPlugin();

    bool detect( QSerialIODevice *device );
    QSerialIODeviceMultiplexer *create( QSerialIODevice *device );

private:
    bool rockwell;
};

class PstnMultiplexer : public QSerialIODeviceMultiplexer
{
    Q_OBJECT
    friend class PstnCommandChannel;
public:
    PstnMultiplexer( QSerialIODevice *device,
			   const QString& switchOut,
			   const QString& switchIn,
			   QObject *parent = 0 );
    ~PstnMultiplexer();

    QSerialIODevice *channel( const QString& name );

private slots:
    void incoming();
    void dataOpened();
    void dataClosed();

private:
    PstnMultiplexerPrivate *d;
};

class PstnCommandChannel : public QSerialIODevice
{
    Q_OBJECT
    friend class PstnMultiplexer;
public:
    PstnCommandChannel( QSerialIODevice *device, QObject *parent = 0 );
    ~PstnCommandChannel();

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

    void add( const char *data, int len );

    void setEnabled( bool value ) { enabled = value; }

private slots:
    void emitReadyRead();

protected:
    qint64 readData( char *data, qint64 maxlen );
    qint64 writeData( const char *data, qint64 len );

private:
    QSerialIODevice *device;
    int used;
    QByteArray buffer;
    bool waitingForReadyRead;
    bool enabled;
};

class PstnDataChannel : public QSerialIODevice
{
    Q_OBJECT
public:
    PstnDataChannel( QSerialIODevice *device, QObject *parent = 0 );
    ~PstnDataChannel();

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
    void abortDial();
    void discard();
    QProcess *run( const QStringList& arguments, bool addPPPdOptions );

    void setEnabled( bool value ) { enabled = value; }

private slots:
    void deviceReadyRead();

protected:
    qint64 readData( char *data, qint64 maxlen );
    qint64 writeData( const char *data, qint64 len );

signals:
    void opened();
    void closed();

private:
    QSerialIODevice *device;
    bool enabled;
};

#endif /* PSTNMULTIPLEXER_H */
