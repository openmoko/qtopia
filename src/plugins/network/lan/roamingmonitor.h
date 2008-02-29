/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef ROAMING_MONITOR
#define ROAMING_MONITOR

#include <custom.h>

#ifndef NO_WIRELESS_LAN

//these two Qt includes must be in fornt of the four system includes for wireless.h
#include <QObject>  
#include <QString>

//must be defined to be able to include kernel includes
#ifndef __user
#define __user
#endif

#include <linux/types.h>    /* required for wireless.h */
#include <sys/socket.h>     /* required for wireless.h */
#include <net/if.h>         /* required for wireless.h */

/* A lot of wireless.h have kernel includes which should be protected by
   #ifdef __KERNEL__. They course include errors due to redefinitions of types.
   This prevents those kernel headers being included by Qtopia.  
   */
#ifndef _LINUX_IF_H
#define _LINUX_IF_H
#endif
#ifndef _LINUX_SOCKET_H
#define _LINUX_SOCKET_H
#endif
#include <linux/wireless.h>

#include <qtopianetworkinterface.h>

class QTimer;
class WirelessScan;
class QValueSpaceItem;
class QSignalSourceProvider;
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
    void updateSignalStrength();

private:
    QtopiaNetworkConfiguration* configIface;
    QTimer* rescanTimer;
    WirelessScan* scanner;
    QValueSpaceItem* netSpace;
    QString deviceName;
    bool activeHop;

    QSignalSourceProvider* signalProvider;
    QTimer* signalTimer;
};

#endif // NO_WIRELESS_LAN
#endif //ROAMING_MONITOR
