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

#ifndef ROAMING_MONITOR
#define ROAMING_MONITOR

#include <custom.h>

#ifndef NO_WIRELESS_LAN
#include <QObject>
#include <QString>

#include <sys/socket.h>
#include <linux/wireless.h>

#include <qtopianetworkinterface.h>

class QTimer;
#if WIRELESS_EXT > 13
class WirelessScan;
#endif
class QValueSpaceItem;

class RoamingMonitor : public QObject
{
    Q_OBJECT
public:
    RoamingMonitor( QtopiaNetworkConfiguration* cfg, QObject* parent = 0 );
    ~RoamingMonitor();

    int selectWLAN( const QString& essid = QString() );
    QString currentEssid() const;
    QString currentMAC() const;
    void activeNotification( bool enabled );

signals:
    void changeNetwork();

private slots:
    void scanTimeout();
    void newScanResults();
    void deviceNameChanged();

private:
    QtopiaNetworkConfiguration* configIface;
    QTimer* rescanTimer;
#if WIRELESS_EXT > 13
    WirelessScan* scanner;
#endif
    QValueSpaceItem* netSpace;
    QString deviceName;
    bool activeHop;
};

#endif // NO_WIRELESS_LAN
#endif //ROAMING_MONITOR
