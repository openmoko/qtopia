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

#include "omap730multiplexer.h"
#include <qmultiportmultiplexer.h>
#include <qserialport.h>
#include <qtopiaglobal.h>

Omap730MultiplexerPlugin::Omap730MultiplexerPlugin( QObject* parent )
    : QSerialIODeviceMultiplexerPlugin( parent )
{
}

Omap730MultiplexerPlugin::~Omap730MultiplexerPlugin()
{
}

bool Omap730MultiplexerPlugin::detect( QSerialIODevice * )
{
    // If this plugin is called, then it is the one we want.
    return true;
}

QSerialIODeviceMultiplexer *Omap730MultiplexerPlugin::create
	( QSerialIODevice *device )
{
    // The primary AT command device, /dev/csmi/5, is configured
    // in the custom.h file as QTOPIA_PHONE_DEVICE and then passed
    // down to us in the "device" parameter.
    QMultiPortMultiplexer *mux = new QMultiPortMultiplexer( device );

    // The secondary channel is the same as the primary channel.
    mux->addChannel( "secondary", device );

    // Add the data channel.
    QSerialPort *data = QSerialPort::create( "/dev/csmi/6" );
    mux->addChannel( "data", data );

    // Add the data setup channel.  Pass "device" as the second argument
    // to use the primary AT command device for data setup.
    mux->addChannel( "datasetup", data );
    return mux;
}

QTOPIA_EXPORT_PLUGIN( Omap730MultiplexerPlugin )
