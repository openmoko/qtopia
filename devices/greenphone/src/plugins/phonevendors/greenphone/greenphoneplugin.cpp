/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/
#include "greenphoneplugin.h"
#include "vendor_greenphone_p.h"

QTOPIA_EXPORT_PLUGIN( GreenphonePluginImpl )

GreenphonePluginImpl::GreenphonePluginImpl()
{
}


GreenphonePluginImpl::~GreenphonePluginImpl()
{
}


bool GreenphonePluginImpl::supports( const QString& manufacturer )
{
    return manufacturer.contains( "Broadcom" );
}

QModemService *GreenphonePluginImpl::create
    ( const QString& service, QSerialIODeviceMultiplexer *mux, QObject *parent )
{
    return new GreenphoneModemService( service, mux, parent );
}
