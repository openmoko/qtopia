/****************************************************************************
**
** Copyright ( C ) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef __QBLUETOOTHREMOTEDEVICEDIALOG_H__
#define __QBLUETOOTHREMOTEDEVICEDIALOG_H__

#include <qbluetoothnamespace.h>
#include <qbluetoothaddress.h>

#include <QDialog>
#include <QSet>


class QBLUETOOTH_EXPORT QBluetoothRemoteDeviceDialogFilter
{
public:
    QBluetoothRemoteDeviceDialogFilter();
    virtual ~QBluetoothRemoteDeviceDialogFilter();

    void setAcceptedDeviceMajors( const QSet<QBluetooth::DeviceMajor> &deviceMajors );
    QSet<QBluetooth::DeviceMajor> acceptedDeviceMajors() const;

    void setAcceptedServiceClasses( QBluetooth::ServiceClasses serviceClasses );
    QBluetooth::ServiceClasses acceptedServiceClasses() const;

    virtual bool filterAcceptsDevice( const QBluetoothRemoteDevice &device );

protected:
    QSet<QBluetooth::DeviceMajor> m_deviceMajors;
    QBluetooth::ServiceClasses m_serviceClasses;

private:
    Q_DISABLE_COPY(QBluetoothRemoteDeviceDialogFilter);
};


class QBluetoothRemoteDeviceDialogPrivate;
class QActionEvent;

class QBLUETOOTH_EXPORT QBluetoothRemoteDeviceDialog : public QDialog
{
    friend class QBluetoothRemoteDeviceDialogPrivate;
    Q_OBJECT

public:
    explicit QBluetoothRemoteDeviceDialog( QWidget *parent = 0,
                                           Qt::WFlags flags = 0 );
    explicit QBluetoothRemoteDeviceDialog( QBluetoothLocalDevice *local = 0,
                                           QWidget *parent = 0,
                                           Qt::WFlags flags = 0 );
    virtual ~QBluetoothRemoteDeviceDialog();

    static QBluetoothAddress getRemoteDevice(
            QWidget *parent = 0,
            QSet<QBluetooth::SDPProfile> profiles = QSet<QBluetooth::SDPProfile>(),
            QBluetoothRemoteDeviceDialogFilter *filter = 0);

    void setValidationProfiles( QSet<QBluetooth::SDPProfile> profiles );
    QSet<QBluetooth::SDPProfile> validationProfiles() const;

    void setFilter( QBluetoothRemoteDeviceDialogFilter *filter );
    QBluetoothRemoteDeviceDialogFilter *filter() const;

    QBluetoothAddress selectedDevice() const;

public slots:
    virtual void done( int r );

protected:
    virtual void actionEvent(QActionEvent *event);

private:
    QBluetoothRemoteDeviceDialogPrivate *m_data;
};

#endif
