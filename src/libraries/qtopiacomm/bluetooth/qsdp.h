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

#ifndef __QSDP_H__
#define __QSDP_H__

#include <qbluetoothnamespace.h>
#include <QString>

class QBluetoothLocalDevice;
class QProcess;
class QSDPService;
class QSDPPrivate;

class QTOPIACOMM_EXPORT QSDP {
public:
    QSDP();
    ~QSDP();

    bool registerService(const QBluetoothLocalDevice &device,
                         QBluetooth::SDPProfile profile, int channel = -1);
    bool unregisterService(const QBluetoothLocalDevice &device,
                           const QSDPService &service);

    QString error();

private:
    QSDPPrivate *m_data;
};

#endif
