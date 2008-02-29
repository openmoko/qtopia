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

#include "wlanregistrationprovider.h"

#ifndef NO_WIRELESS_LAN

#include <QAbstractIpcInterfaceGroup>

class WlanRegistrationInterface : public QWlanRegistration
{
    Q_OBJECT
public:

    WlanRegistrationInterface( const QString& serviceName, QObject* parent )
        : QWlanRegistration( serviceName, parent, QAbstractIpcInterface::Server )
    {
        setValue( "currentESSID", "" );
    }

    void setNewAccessPoint( const QString& essid )
    {
        if ( essid != value("currentESSID").toString() ) {
            setValue( "currentESSID", essid );
            emit accessPointChanged(); 
        }
    }
};

/*
   This class provides the back-end support for QWlanRegistration. It monitors the current ESSID
   and MAC and sends notifications when they change.
   */
WlanRegistrationProvider::WlanRegistrationProvider( const QString& serviceName, QObject* parent )
    : QAbstractIpcInterfaceGroup( serviceName, parent ), 
        wri(0), servName(serviceName)
{
}

WlanRegistrationProvider::~WlanRegistrationProvider()
{
}

/*
   Initializes this wlan provider
   */
void WlanRegistrationProvider::initialize()
{
    if ( !supports<QWlanRegistration>() ) {
        wri = new WlanRegistrationInterface( servName, this );
        addInterface( wri );
    }
    
    QAbstractIpcInterfaceGroup::initialize(); 
}

void WlanRegistrationProvider::setAccessPoint( const QString& sid )
{
    essid = sid;
}

void WlanRegistrationProvider::notifyClients()
{
    if ( wri )
        wri->setNewAccessPoint( essid );
}

#include "wlanregistrationprovider.moc"

#endif //NO_WIRELESS_LAN
