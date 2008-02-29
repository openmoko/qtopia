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
#include "serialport.h"

#include <trace.h>
QD_LOG_OPTION(Modem)

#include <QTimer>

SerialPort::SerialPort( const QString &port )
    : QSerialPort( port, 9600 ), up( false )
{
    connect( this, SIGNAL(dsrChanged(bool)), this, SLOT(slotDsrChanged(bool)) );
    connect( this, SIGNAL(readyRead()), this, SLOT(slotReadyRead()) );
}

SerialPort::~SerialPort()
{
    TRACE(Modem) << "SerialPort::~SerialPort";
}

void SerialPort::slotDsrChanged( bool dsrUp )
{
    TRACE(Modem) << "SerialPort::slotDsrChanged" << "DSR" << dsrUp << "up" << up;
    if ( dsrUp ) {
        if ( !up ) {
            up = true;
            emit newConnection();
        }
    } else {
        if ( up ) {
            emit disconnected();
            up = false;
        }
    }
}

bool SerialPort::open( QIODevice::OpenMode mode )
{
    TRACE(Modem) << "SerialPort::open";
    bool ret;

    ret = QSerialPort::open( mode );
    up = false;

    if ( ret && dsr() ) {
        up = true;
        QTimer::singleShot( 0, this, SIGNAL(newConnection()) );
    }

    return ret;
}

void SerialPort::slotReadyRead()
{
    TRACE(Modem) << "SerialPort::slotReadyRead";
    // We might not get a DSR so bring up the connection now
    LOG() << "up" << up;
    if ( !up ) {
        up = true;
        LOG() << "coming up (data received), emit newConnection";
        emit newConnection();
        // We need to emit this again so the QIODeviceWrapper can see it!
        emit readyRead();
    }
    // We've brought the connection up so stop listening for the signal
    disconnect( this, SIGNAL(readyRead()), this, SLOT(slotReadyRead()) );
}

