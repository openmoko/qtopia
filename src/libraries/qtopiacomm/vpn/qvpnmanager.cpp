/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#include "qvpnmanager_p.h"

#include "qvpnfactory.h"
#include "qvpnclient.h"
#include "qkey.h"

#include <QDebug>

#include <qtopiaipcadaptor.h>
#include <qcategorymanager.h>
#include <qtopialog.h>
#include <qtopianamespace.h>

QVpnManager::QVpnManager( QObject* parent )
    : QtopiaIpcAdaptor( QLatin1String("QPE/VPNManager"), parent )
{
    publishAll( QtopiaIpcAdaptor::Slots );
    QString certificateString=QLatin1String("Certificate"),
            securityKeyString=QLatin1String("Security Key");

    // Ensure these system categories exist
    QCategoryManager cats("Documents");
    cats.addCategory(certificateString, certificateString, QString(), false, true);
    if(!cats.isSystem(certificateString))
        cats.setSystem(certificateString);
    cats.addCategory(securityKeyString, securityKeyString, QString(), false, true);
    if(!cats.isSystem(securityKeyString))
        cats.setSystem(securityKeyString);

    vpnFactory = new QVPNFactory();
    vpnFactory->setServerMode( true );

    qLog(VPN) << "Starting VPN Manager";
}

QVpnManager::~QVpnManager()
{
    if ( idToVPN.count() > 0 ) {
        QMutableHashIterator<uint,QVPNClient*> i(idToVPN);
        QVPNClient* vpn = 0;
        while( i.hasNext() ) {
            i.next();
            vpn = i.value();
            if ( vpn && vpn->state() != QVPNClient::Disconnected ) {
                qLog(VPN) << "Stopping VPN" << vpn->name();
                vpn->disconnect();
                i.remove();
                delete vpn;
                vpn = 0;
            }
        }
    }

    delete vpnFactory;
    vpnFactory = 0;
}

void QVpnManager::connectVPN( uint vpnID )
{
    qLog(VPN) << "VPNManager received request to start VPN" << vpnID;
    QVPNClient* vpn = 0;
    if ( idToVPN.contains(vpnID) ) {
        vpn = idToVPN[ vpnID ];
    } else {
        vpn = vpnFactory->create( vpnID, this );
        idToVPN[vpnID] = vpn;
    }

    if ( vpn->state() == QVPNClient::Disconnected ) {
        qLog(VPN) << "Starting VPN connection" << vpn->name();
        vpn->connect();
    } else {
        qLog(VPN) << vpn->name() << "is already running.";
    }

}

void QVpnManager::disconnectVPN( uint vpnID )
{
    qLog(VPN) << "VPNManager received request to stop VPN" << vpnID;
    if ( idToVPN.contains(vpnID) ) {
        QVPNClient* vpn = idToVPN[vpnID ];
        if ( !vpn )
            return;
        vpn->disconnect();
        qLog(VPN) << "Stopping VPN connection" << vpn->name();
    } else {
        qLog(VPN) << "VPN" << vpnID << "is not running.";
    }
}

void QVpnManager::deleteVPN( uint vpnID )
{
    qLog(VPN) << "VPNManager received request to delete VPN" << vpnID;
    QVPNClient* vpn = 0;
    if ( idToVPN.contains(vpnID) )
        vpn = idToVPN.take( vpnID );
    else
        return;

    if ( !vpn )
        return;

    if ( vpn->state() != QVPNClient::Disconnected ) {
        qLog(VPN) << "VPN is still running. Stop VPN and try again";
        return;
    }

    qLog(VPN) << "VPN deleted" ;
    delete vpn;
    vpn = 0;
}

