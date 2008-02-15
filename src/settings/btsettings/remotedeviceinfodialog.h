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
#ifndef __REMOTEDEVICEINFODIALOG_H__
#define __REMOTEDEVICEINFODIALOG_H__

#include <qbluetoothaddress.h>
#include <qbluetoothremotedevice.h>

#include <QDialog>
#include <QVariant>

namespace Ui {
    class RemoteDeviceInfo;
};

class QBluetoothAddress;
class QBluetoothLocalDevice;
class QBluetoothRemoteDevice;
class AudioDeviceConnectionStatus;
class GenericDeviceConnectionStatus;

class RemoteDeviceInfoDialog : public QDialog
{
    Q_OBJECT

public:
    RemoteDeviceInfoDialog(QBluetoothLocalDevice *local, QWidget *parent, Qt::WFlags flags = 0);
    ~RemoteDeviceInfoDialog();

    void setRemoteDevice(const QBluetoothAddress &addr);
    void setRemoteDevice(const QBluetoothRemoteDevice &device);

private slots:
    void showMoreInfo();
    void showServices();
    void setTitle();

private:
    Ui::RemoteDeviceInfo *m_ui;
    QBluetoothLocalDevice *m_local;

    QBluetoothRemoteDevice m_device;
    QBluetoothAddress m_address;
    QString m_name;
    QVariant m_nameFontVariant;

    AudioDeviceConnectionStatus *m_audioDeviceConnStatus;
    GenericDeviceConnectionStatus *m_genericDeviceConnStatus;
};

#endif
