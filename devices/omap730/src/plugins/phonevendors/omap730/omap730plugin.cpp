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
#include "omap730plugin.h"
#include "vendor_omap730_p.h"

QTOPIA_EXPORT_PLUGIN( Omap730PluginImpl )

Omap730PluginImpl::Omap730PluginImpl()
{
}


Omap730PluginImpl::~Omap730PluginImpl()
{
}


bool Omap730PluginImpl::supports( const QString& manufacturer )
{
    return manufacturer.contains( "Texas Instruments" );
}

QModemService *Omap730PluginImpl::create
    ( const QString& service, QSerialIODeviceMultiplexer *mux, QObject *parent )
{
    return new Omap730ModemService( service, mux, parent );
}
