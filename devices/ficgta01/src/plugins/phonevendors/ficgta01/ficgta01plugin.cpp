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
#include "ficgta01plugin.h"
#include "vendor_ficgta01_p.h"

QTOPIA_EXPORT_PLUGIN( Ficgta01PluginImpl )

Ficgta01PluginImpl::Ficgta01PluginImpl()
{
}


Ficgta01PluginImpl::~Ficgta01PluginImpl()
{
}


bool Ficgta01PluginImpl::supports( const QString& manufacturer )
{
    return  manufacturer.contains( "OpenMoko" );
    // "Neo1973 GTA01 Embedded GSM Modem"
    //
}

QModemService *Ficgta01PluginImpl::create
    ( const QString& service, QSerialIODeviceMultiplexer *mux, QObject *parent )
{
    return new Ficgta01ModemService( service, mux, parent );
}
