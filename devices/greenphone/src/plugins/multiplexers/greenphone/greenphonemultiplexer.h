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

#ifndef GREENPHONEMUTLIPLEXER_H
#define GREENPHONEMUTLIPLEXER_H

#include <qserialiodevicemultiplexerplugin.h>
#include <qserialiodevicemultiplexer.h>
#include <qgsm0710multiplexer.h>

class QTimer;
class GreenphoneMultiplexer;

class GreenphoneMultiplexerPlugin : public QSerialIODeviceMultiplexerPlugin
{
    Q_OBJECT
public:
    GreenphoneMultiplexerPlugin( QObject* parent = 0 );
    ~GreenphoneMultiplexerPlugin();

    bool detect( QSerialIODevice *device );
    QSerialIODeviceMultiplexer *create( QSerialIODevice *device );
};

class QGreenphoneWakeupSerialIODevice : public QSerialIODevice
{
    Q_OBJECT
    friend class GreenphoneMultiplexer;
public:
    QGreenphoneWakeupSerialIODevice
        ( QSerialIODevice *device, QObject *parent = 0 );
    ~QGreenphoneWakeupSerialIODevice();

    bool open( OpenMode mode );
    void close();
    bool waitForReadyRead(int msecs);
    qint64 bytesAvailable() const;

    int rate() const;
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

protected:
    qint64 readData( char *data, qint64 maxlen );
    qint64 writeData( const char *data, qint64 len );

private slots:
    void sleep();

    void haveReady();
    void received(const QString &message, const QByteArray &data);

private:
    QSerialIODevice *device;
    int wakeupFd;
    bool modemAsleep;
    QTimer *sleepTimer;
    GreenphoneMultiplexer *mux;

    void wakeup(bool delay = false);
    void forceWakeup();
};

class GreenphoneMultiplexer : public QGsm0710Multiplexer
{
    Q_OBJECT
public:
    GreenphoneMultiplexer( QGreenphoneWakeupSerialIODevice *device,
                           int frameSize = 31, bool advanced = false,
                           QObject *parent = 0 );
    ~GreenphoneMultiplexer();

    void callReinit() { reinit(); }
};


#endif /* GREENPHONEMUTLIPLEXER_H */
