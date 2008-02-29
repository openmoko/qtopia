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

#ifndef __QBLUETOOTHDEVICESELECTOR_H__
#define __QBLUETOOTHDEVICESELECTOR_H__

#include <qbluetoothnamespace.h>
#include <qbluetoothaddress.h>

#include <QDialog>
#include <QSet>


class QTOPIACOMM_EXPORT QBluetoothDeviceSelectorFilter
{
public:
    QBluetoothDeviceSelectorFilter();
    QBluetoothDeviceSelectorFilter( const QBluetoothDeviceSelectorFilter &other );
    virtual ~QBluetoothDeviceSelectorFilter();

    QBluetoothDeviceSelectorFilter &operator=(const QBluetoothDeviceSelectorFilter &other);
    bool operator==(const QBluetoothDeviceSelectorFilter &other);

    void setAcceptedDeviceMajors( const QSet<QBluetooth::DeviceMajor> &deviceMajors );
    QSet<QBluetooth::DeviceMajor> acceptedDeviceMajors() const;

    void setAcceptedServiceClasses( QBluetooth::ServiceClasses serviceClasses );
    QBluetooth::ServiceClasses acceptedServiceClasses() const;

    virtual bool filterAcceptsDevice( const QBluetoothRemoteDevice &device );

protected:
    QSet<QBluetooth::DeviceMajor> m_deviceMajors;
    QBluetooth::ServiceClasses m_serviceClasses;
};

class QBluetoothDeviceSelector_Private;

class QTOPIACOMM_EXPORT QBluetoothDeviceSelector : public QDialog
{
    friend class QBluetoothDeviceSelector_Private;
    Q_OBJECT

public:
    explicit QBluetoothDeviceSelector( bool loadFavoritesOnShow, QWidget *parent = 0,
                                       Qt::WFlags flags = 0 );
    explicit QBluetoothDeviceSelector( QWidget *parent = 0, Qt::WFlags flags = 0 );
    ~QBluetoothDeviceSelector();

    static QBluetoothAddress getRemoteDevice(
            QSet<QBluetooth::SDPProfile> profiles = QSet<QBluetooth::SDPProfile>(),
            const QBluetoothDeviceSelectorFilter &filter = QBluetoothDeviceSelectorFilter(),
            bool loadFavoritesOnShow = true,
            QWidget *parent = 0 );

    void setValidationProfiles( QSet<QBluetooth::SDPProfile> profiles );
    QSet<QBluetooth::SDPProfile> validationProfiles() const;

    void setForcePairing( bool force );
    bool forcePairing() const;

    void setFilter( const QBluetoothDeviceSelectorFilter &filter );
    QBluetoothDeviceSelectorFilter filter() const;

    void addDeviceAction( QAction *action );
    void removeDeviceAction( QAction *action );

    QList<QBluetoothAddress> displayedDevices() const;
    QBluetoothAddress selectedDevice() const;

    bool startDeviceDiscovery( bool clear );


signals:
    void deviceSelectionChanged();
    void deviceActivated( const QBluetoothAddress &addr );

    void deviceDiscoveryStarted();
    void deviceDiscoveryCompleted();

private:
    QBluetoothDeviceSelector_Private *m_private;
};

#endif
