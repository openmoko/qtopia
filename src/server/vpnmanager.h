/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef VPN_MANAGER_H
#define VPN_MANAGER_H

#ifdef QTOPIA_VPN

#include <qtopiaipcadaptor.h>
#include <QHash>

class QVPNClient;
class QVPNFactory;

class QtopiaVpnManager : public QtopiaIpcAdaptor {
    Q_OBJECT
public:
    explicit QtopiaVpnManager( QObject* parent = 0 );
    ~QtopiaVpnManager();

public slots:
    void connectVPN( uint vpnID );
    void disconnectVPN( uint vpnID );
    void deleteVPN( uint vpnID ) ;

private:
    QVPNFactory* vpnFactory;
    QHash<uint,QVPNClient*> idToVPN;
};
#endif //QTOPIA_VPN

#endif //VPN_MANAGER_H
