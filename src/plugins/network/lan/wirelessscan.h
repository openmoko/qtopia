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

#ifndef WIRELESS_SCAN
#define WIRELESS_SCAN

#include <custom.h>
#include "wnet.h"
#ifndef NO_WIRELESS_LAN

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

#include <QObject>
#include <QWidget>
#include <QList>

#include <qtopianetworkinterface.h>

class WirelessScan : public QObject {
    Q_OBJECT
public:
    enum ConnectionState {
        InterfaceUnavailable,
        NotConnected,
        Connected
    };

    WirelessScan( const QString& ifaceName, QObject* parent = 0 );
    virtual ~WirelessScan();

    QString attachedInterface() const { return iface; };
    const QList<WirelessNetwork> results() const;

    QString currentAccessPoint() const;
    QString currentESSID() const;
    int currentSignalStrength() const;
    ConnectionState deviceState() const;
    bool isScanning() { return sockfd != -1; } ;

    void rangeInfo( struct iw_range* range, int* weVersion ) const;

public slots:
    bool startScanning();
signals:
    void scanningFinished();
private slots:
    void checkResults();
private:
    void readData( unsigned char* data, int length, int weVersion, struct iw_range* range );
    void ensureScanESSID();

    QString iface;
    QList<WirelessNetwork> entries;
    int sockfd;
};

class QLabel;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QNetworkDevice;

class WSearchPage : public QWidget
{
    Q_OBJECT
public:
    WSearchPage( const QString& config, QWidget* parent = 0, Qt::WFlags flags = 0 );
    virtual ~WSearchPage();

    void attachToInterface( const QString& ifaceName );
    void saveScanResults();

public slots:
    void updateConnectivity();

private slots:
    void updateActions( QListWidgetItem* cur, QListWidgetItem* prev);
    void connectToNetwork();
    void deleteNetwork();
    void showAllNetworks();
    void stateChanged(QtopiaNetworkInterface::Status newState, bool error);
    void changePriority( QListWidgetItem* item );

private:
    void initUI();
    void loadKnownNetworks();
    void saveKnownNetworks();
    void updateKnownNetworkList( const WirelessNetwork& record, QListWidgetItem* item  = 0);

private:
    QString config;
    WirelessScan* scanEngine;
#ifdef QTOPIA_KEYPAD_NAVIGATION
    QAction* connectAction, *scanAction, *environmentAction, *deleteAction;
#else
    QPushButton *connectPB, *refreshPB;
#endif
    QListWidget* knownNetworks;
    QLabel* descr;
    QLabel* currentNetwork;
    QtopiaNetworkInterface::Status state;
    bool isShiftMode;
    bool isRestart;
    QNetworkDevice *devSpace;
};
#endif
#endif
