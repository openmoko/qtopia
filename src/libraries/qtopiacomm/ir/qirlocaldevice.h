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

#ifndef __QIRLOCALDEVICE_H__
#define __QIRLOCALDEVICE_H__

#include <qobject.h>
#include <qglobal.h>
#include <QString>

#include <qirglobal.h>
#include <qirnamespace.h>

class QIrIasAttribute;
class QIrLocalDevice_Private;
class QIrRemoteDevice;

class QIR_EXPORT QIrLocalDevice : public QObject
{
    Q_OBJECT

    friend class QIrLocalDevice_Private;

public:
    explicit QIrLocalDevice(const QString &device);
    ~QIrLocalDevice();

    const QString &deviceName() const;

    bool isUp() const;
    bool bringUp();
    bool bringDown();

    static QStringList devices();

    QVariant queryRemoteAttribute(const QIrRemoteDevice &remote,
                                  const QString &className,
                                  const QString &attribName);

public slots:
    bool discoverRemoteDevices(QIr::DeviceClasses classes = QIr::All);

signals:
    void discoveryStarted();
    void remoteDeviceFound(const QIrRemoteDevice &device);
    void remoteDevicesFound(const QList<QIrRemoteDevice> &devices);
    void discoveryCompleted();

private:
    QIrLocalDevice_Private *m_data;
    Q_DISABLE_COPY(QIrLocalDevice)
};

#endif
