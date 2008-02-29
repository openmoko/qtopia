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

#ifndef __QIRREMOTEDEVICE_H__
#define __QIRREMOTEDEVICE_H__

#include <qirnamespace.h>
#include <qirglobal.h>

#include <QString>

class QIR_EXPORT QIrRemoteDevice
{
public:
    QIrRemoteDevice(const QString &name,
                    QIr::DeviceClasses &devClasses,
                    uint addr);
    QIrRemoteDevice(const QIrRemoteDevice &dev);
    ~QIrRemoteDevice();

    QIrRemoteDevice &operator=(const QIrRemoteDevice &other);
    bool operator==(const QIrRemoteDevice &other) const;

    uint address() const;
    QIr::DeviceClasses deviceClasses() const;
    QString name() const;

private:
    QString m_name;
    QIr::DeviceClasses m_dev_class;
    uint m_addr;
};

#endif
