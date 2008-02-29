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
#ifndef __QBLUETOOTHREMOTEDEVICEDIALOG_P_H__
#define __QBLUETOOTHREMOTEDEVICEDIALOG_P_H__

#include "qbluetoothremotedevicedialog.h"
#include <qbluetoothremotedevice.h>
#include <qbluetoothlocaldevice.h>
#include <qbluetoothsdprecord.h>
#include <qtopiacomm/private/qbluetoothpairingagent_p.h>

#include <QWidget>
#include <QHash>
#include <QList>
#include <QPixmap>
#include <QIcon>
#include <QSettings>


class QTableWidget;
class QTableWidgetItem;
class QAction;
class QLabel;
class QTimer;
class QPixmap;
class QWaitWidget;
class QBluetoothSdpQuery;
class QShowEvent;
class QBluetoothSdpQueryResult;
class QMenu;
class QSettings;
class QDBusInterface;

class RemoteDevicePropertiesDialog;


class DevicesTableController : public QObject
{
    Q_OBJECT

public:
    DevicesTableController( QBluetoothRemoteDeviceDialogPrivate *parent = 0 );
    ~DevicesTableController();

    void addDevice( const QBluetoothRemoteDevice &remote,
                    const QBluetoothLocalDevice &local );
    void refreshDevice( int row, const QBluetoothLocalDevice &local, const QString &updatedName = QString() );
    void clear();
    int deviceCount() const;

    QBluetoothAddress deviceAt( int row ) const;
    QBluetoothRemoteDevice deviceObjectAt( int row ) const;
    QString deviceLabelAt( int row ) const;   // could be alias or name
    int row( const QBluetoothAddress &addr ) const;

    QBluetoothAddress selectedDevice() const;

    void setPaired( int row, bool paired );

    QTableWidget *devicesTable() const;
    void redrawTable();
    void showDiscoveryButton();

private slots:
    void cellActivated( int row, int col );
    void currentCellChanged( int row, int col, int prevRow, int prevCol);

private:
    static const Qt::ItemFlags ITEM_FLAGS;

    QBluetoothRemoteDeviceDialogPrivate *m_parent;
    QTableWidget *m_devicesTable;
    QList<QBluetoothRemoteDevice> m_displayedDevices;
    int m_lastSelectedRow;
    QTableWidgetItem *m_discoveryButton;
};


class BluetoothIconBlinker : public QObject
{
    Q_OBJECT
public:
    BluetoothIconBlinker( int blinkInterval, QObject *parent = 0 );
    ~BluetoothIconBlinker();

    void setBlinking( bool blink );
    QLabel *iconLabel() const;

private slots:
    void toggleIconImage();

private:
    QLabel *m_icon;
    QTimer *m_timer;
    QPixmap m_pixmapOnline;
    QPixmap m_pixmapOffline;
};


class QBluetoothRemoteDeviceDialogPrivate : public QWidget
{
    friend class DevicesTableController;
    Q_OBJECT

public:
    QBluetoothRemoteDeviceDialogPrivate( const QBluetoothAddress &localAddr,
                                      bool showPairedAndFavorites,
                                      QBluetoothRemoteDeviceDialog *parent );
    ~QBluetoothRemoteDeviceDialogPrivate();

    void setValidationProfiles( QSet<QBluetooth::SDPProfile> profiles );
    QSet<QBluetooth::SDPProfile> validationProfiles() const;

    void setFilter( const QBluetoothRemoteDeviceDialogFilter &filter );
    QBluetoothRemoteDeviceDialogFilter filter() const;

    void addDeviceAction( QAction *action, bool isExternalAction );
    void removeDeviceAction( QAction *action );

    void showEvent( QShowEvent *e );

    QBluetoothAddress selectedDevice() const;

    bool startDeviceDiscovery( bool clear );

    void cleanUp();

signals:
    void deviceActivatedOk();

private slots:
    void discoveryActionTriggered();
    void discoveredDevice( const QBluetoothRemoteDevice &device );
    void completedDeviceDiscovery();
    void deviceDiscoveryCancelled();
    void remoteNameUpdated(const QBluetoothAddress &, const QString &);

    void localDeviceError( QBluetoothLocalDevice::Error error, const QString &message );

    void pairingRequested( int row );
    void unpairingRequested( int row );
    void performPairingOperation( int row, QBluetoothPairingAgent::Operation op );

    void foundServices( const QBluetoothSdpQueryResult &result );
    void serviceSearchCancelled();
    void deviceValidationCancelled();
    void validateServices();

    void favoritesActionTriggered();
    void showRemoteProperties();

    void updateDeviceDisplay( const QBluetoothAddress &addr );

    void changedDeviceSelection();
    void activatedDevice( int row );

private:
    void cancelDeviceDiscovery();
    void deviceValidationCompleted();
    bool serviceProfilesMatch( const QList<QBluetoothSdpRecord> services,
                               QSet<QBluetooth::SDPProfile> profiles );

    void enableDeviceActions(bool enable);
    void addDeviceToDisplay( const QBluetoothRemoteDevice &device );
    void clearDevices();

    void addFavorite( const QBluetoothAddress &addr );
    void removeFavorite( const QBluetoothAddress &addr );
    QList<QBluetoothAddress> favorites();
    bool isFavorite( const QBluetoothAddress &addr );
    void loadPairedAndFavorites();

    void initParentUI();
    void initWidgets();
    void initActions();

    static const QString SETTINGS_FAVORITES_KEY;

    QBluetoothRemoteDeviceDialog *m_parent;

    QBluetoothLocalDevice *m_local;
    bool m_showPairedAndFavorites;
    QBluetoothSdpQuery *m_sdap;
    QSet<QBluetooth::SDPProfile> m_validProfiles;
    QBluetoothRemoteDeviceDialogFilter m_filter;
    DevicesTableController *m_devicesTableController;
    bool m_neverShownBefore;

    bool m_discovering;
    bool m_cancelledDiscovery;
    bool m_forcingDiscovery;
    QBluetoothAddress m_deviceUnderValidation;

    QMenu *m_menu;
    QList<QAction *> m_deviceActions;
    QAction *m_discoveryAction;
    QAction *m_favoritesAction;
    QAction *m_lastPersistentAction;
    QAction *m_deviceActionsSeparator;
    QIcon m_searchIcon;
    QIcon m_stopSearchIcon;

    BluetoothIconBlinker *m_bluetoothStatusIcon;
    QLabel *m_statusLabel;

    // this also used to determine whether a validation is
    // currently in process, so make sure to call show() before starting
    // validation and hide() when it's done
    QWaitWidget *m_validationWaitWidget;

    QSettings m_settings;
    QHash<QString, bool> m_favorites;

    RemoteDevicePropertiesDialog *m_deviceInfoDialog;

    bool m_cancellingSearch;
};

#endif
