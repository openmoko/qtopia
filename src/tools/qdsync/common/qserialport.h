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
#ifndef Q_QDOC
#ifndef QSERIALPORT_H
#define QSERIALPORT_H

#include <qdglobal.h>
#include "qserialiodevice.h"

namespace QDSync {

class QSerialPortPrivate;

class QD_EXPORT QSerialPort : public QSerialIODevice
{
    Q_OBJECT
public:
    explicit QSerialPort( const QString& device, int rate = 38400, bool trackStatus = false );
    ~QSerialPort();

    // Override QIODevice methods.
    bool open( OpenMode mode );
    void close();
    bool flush();
    bool waitForReadyRead(int msecs);
    qint64 bytesAvailable() const;

    // Get or set the CTS/RTS flow control mode.
    bool flowControl() const;
    void setFlowControl( bool value );

    bool keepOpen() const;
    void setKeepOpen( bool value );

    // Override QSerialIODevice methods.
    int rate() const;
    bool dtr() const;
    void setDtr( bool value );
    bool dsr() const;
    bool carrier() const;
    bool rts() const;
    void setRts( bool value );
    bool cts() const;
    void discard();
    bool isValid() const;
    QProcess *run( const QStringList& arguments, bool addPPPdOptions );

    // Create and open a serial device from a "device:rate" name.
    static QSerialPort *create( const QString& name, int defaultRate=115200,
                                bool flowControl=false );

#ifndef Q_OS_UNIX
    void setBrokenSerial();
#endif

protected:
    qint64 readData( char *data, qint64 maxlen );
    qint64 writeData( const char *data, qint64 len );

private slots:
    void statusTimeout();
    void pppdStateChanged( QProcess::ProcessState state );
    void pppdDestroyed();
    void socketReady();

private:
    QSerialPortPrivate *d;
};

};

using namespace QDSync;

#endif // QSERIALPORT_H
#endif
