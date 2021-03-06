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

#include <qtopiacomm/private/qpassthroughserialiodevice_p.h>

// The QPassThroughSerialIODevice class passes all data through to
// an underlying device, but only if the "enabled" flag is set.  This
// allows multiple pass-through devices to use the same underlying
// device, with switching between them.  The most common use for this
// is to implement pseudo-multiplexing on non-multiplexing modems.
// When data mode needs to be entered, the primary command channel
// "disconnects" from the underlying device so the data channel can use it.

QPassThroughSerialIODevice::QPassThroughSerialIODevice
        ( QSerialIODevice *device, QObject *parent )
    : QSerialIODevice( parent )
{
    this->device = device;
    this->enabled = false;

    connect( device, SIGNAL(readyRead()), this, SLOT(deviceReadyRead()) );
}

QPassThroughSerialIODevice::~QPassThroughSerialIODevice()
{
    // Nothing to do here.
}

bool QPassThroughSerialIODevice::open( OpenMode mode )
{
    setOpenMode( mode | QIODevice::Unbuffered );
    emit opened();
    return true;
}

void QPassThroughSerialIODevice::close()
{
    setOpenMode( NotOpen );
    emit closed();
}

bool QPassThroughSerialIODevice::waitForReadyRead(int msecs)
{
    if ( enabled )
        return device->waitForReadyRead( msecs );
    else
        return false;
}

qint64 QPassThroughSerialIODevice::bytesAvailable() const
{
    if ( enabled )
        return device->bytesAvailable();
    else
        return 0;
}

int QPassThroughSerialIODevice::rate() const
{
    if ( enabled )
        return device->rate();
    else
        return 115200;
}

bool QPassThroughSerialIODevice::dtr() const
{
    if ( enabled )
        return device->dtr();
    else
        return true;
}

void QPassThroughSerialIODevice::setDtr( bool value )
{
    if ( enabled )
        device->setDtr( value );
}

bool QPassThroughSerialIODevice::dsr() const
{
    if ( enabled )
        return device->dsr();
    else
        return true;
}

bool QPassThroughSerialIODevice::carrier() const
{
    if ( enabled )
        return device->carrier();
    else
        return true;
}

bool QPassThroughSerialIODevice::setCarrier( bool value )
{
    if ( enabled )
        return device->setCarrier( value );
    else
        return false;
}

bool QPassThroughSerialIODevice::rts() const
{
    if ( enabled )
        return device->rts();
    else
        return true;
}

void QPassThroughSerialIODevice::setRts( bool value )
{
    if ( enabled )
        device->setRts( value );
}

bool QPassThroughSerialIODevice::cts() const
{
    if ( enabled )
        return device->cts();
    else
        return true;
}

void QPassThroughSerialIODevice::discard()
{
    if ( enabled )
        device->discard();
}

bool QPassThroughSerialIODevice::isValid() const
{
    if ( enabled )
        return device->isValid();
    else
        return false;
}

qint64 QPassThroughSerialIODevice::readData( char *data, qint64 maxlen )
{
    if ( enabled )
        return device->read( data, maxlen );
    else
        return 0;
}

qint64 QPassThroughSerialIODevice::writeData( const char *data, qint64 len )
{
    if ( enabled )
        return device->write( data, len );
    else
        return len;
}

void QPassThroughSerialIODevice::deviceReadyRead()
{
    if ( enabled )
        internalReadyRead();
}
