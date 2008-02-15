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

#ifndef QVPNFACTORY
#define QVPNFACTORY

#include "qvpnclient.h"

#include <QSet>

class QTOPIACOMM_EXPORT QVPNFactory {
public:
    QVPNFactory();

    QVPNClient* instance( uint vpnID,  QObject* parent = 0 );
    QVPNClient* create( QVPNClient::Type type, QObject* parent = 0 );

    static QSet<QVPNClient::Type> types();
    static QString name( uint vpnID );
    static QSet<uint> vpnIDs();
protected:
    void setServerMode( bool enable );

private:
    bool serverMode;

    friend class QtopiaVpnManager;
};

#endif //QVPNFACTORY
