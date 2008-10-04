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

#include "iaxservicechecker.h"
#include "iaxtelephonyservice.h"
#include <QSettings>

IaxServiceChecker::IaxServiceChecker( IaxTelephonyService *service )
    : QServiceChecker( service->service(), service, Server )
{
    updateRegistrationConfig();
}

IaxServiceChecker::~IaxServiceChecker()
{
}

void IaxServiceChecker::updateRegistrationConfig()
{
    QSettings config( "Trolltech", "Asterisk" );
    config.beginGroup( "Registration" );
    if ( config.value( "Server" ).toString().isEmpty() )
        setValid( false );
    else
        setValid( true );
}
