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

#include <qtopiaipcadaptor.h>
#include <QHash>

class QVPNClient;
class QVPNFactory;

class QVpnManager : public QtopiaIpcAdaptor {
    Q_OBJECT
public:
    QTOPIACOMM_EXPORT explicit QVpnManager( QObject* parent = 0 );
    QTOPIACOMM_EXPORT ~QVpnManager();

public slots:
    void connectVPN( uint vpnID );
    void disconnectVPN( uint vpnID );
    void deleteVPN( uint vpnID ) ;

private:
    QVPNFactory* vpnFactory;
    QHash<uint,QVPNClient*> idToVPN;
};
