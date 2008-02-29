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

#include "pstnplugin.h"
#include "vendor_pstn_p.h"

QTOPIA_EXPORT_PLUGIN( PstnPluginImpl )

PstnPluginImpl::PstnPluginImpl()
{
}

PstnPluginImpl::~PstnPluginImpl()
{
}

bool PstnPluginImpl::supports( const QString& manufacturer )
{
    // AT+CGMI will never return anything interesting for this modem type
    // because such modems are not GSM-compatible.  The plugin will need
    // to be explicitly enabled using QTOPIA_PHONE_VENDOR.
    Q_UNUSED(manufacturer);
    return false;
}

QModemService *PstnPluginImpl::create
    ( const QString& service, QSerialIODeviceMultiplexer *mux, QObject *parent )
{
    return new PstnModemService( service, mux, parent );
}
