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

#include "multiportmultiplexer.h"
#include <qmultiportmultiplexer.h>
#include <qserialport.h>

bool MultiPortMultiplexerPlugin::detect( QSerialIODevice * )
{
    return true;
}

QSerialIODeviceMultiplexer *MultiPortMultiplexerPlugin::create( QSerialIODevice *device )
{
    // The primary AT command device, /dev/mux0, is configured
    // in the custom.h file as QTOPIA_PHONE_DEVICE and then passed
    // down to us in the "device" parameter.
    QMultiPortMultiplexer *mux = new QMultiPortMultiplexer( device );

    // Add the secondary channel.
    QSerialPort *secondary = QSerialPort::create( "/dev/mux1" );
    mux->addChannel( "secondary", secondary );

    // Add the data channel.
    QSerialPort *data = QSerialPort::create( "/dev/mux2" );
    mux->addChannel( "data", data );

    // Add the data setup channel, which is the same as "data".
    mux->addChannel( "datasetup", data );
    return mux;
}

QTOPIA_EXPORT_PLUGIN( MultiPortMultiplexerPlugin )
