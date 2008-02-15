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

#include "iaxconfiguration.h"
#include "iaxtelephonyservice.h"

IaxConfiguration::IaxConfiguration( IaxTelephonyService *service )
    : QTelephonyConfiguration( service->service(), service, Server )
{
    this->service = service;
}

IaxConfiguration::~IaxConfiguration()
{
}

void IaxConfiguration::update( const QString& name, const QString& )
{
    // Process messages from the "iaxsettings" program for config updates.
    if ( name == "registration" )
        service->updateRegistrationConfig();
    else if ( name == "callerid" )
        service->updateCallerIdConfig();
}

void IaxConfiguration::request( const QString& name )
{
    // Not supported - just return an empty value.
    emit notification( name, QString() );
}
