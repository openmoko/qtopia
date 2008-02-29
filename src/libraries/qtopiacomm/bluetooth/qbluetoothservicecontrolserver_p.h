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

#ifndef __QBLUETOOTHSERVICECONTROLSERVER_H__
#define __QBLUETOOTHSERVICECONTROLSERVER_H__

#include <qtopiacomm/qbluetoothservicecontrol.h>
#include "qbluetoothabstractservice.h"

class QBluetoothServiceControlServer_Private;
class QBluetoothServiceControlServer: public QBluetoothServiceControl
{
    friend class QBluetoothAbstractService;
    friend class QBluetoothServiceControlServer_Private;

public:
    explicit QBluetoothServiceControlServer(QBluetoothAbstractService *service,
                                            QObject *parent = 0);
    virtual ~QBluetoothServiceControlServer();

public slots:
    virtual void start(int channel);
    virtual void stop();
    virtual void setSecurityOptions(QBluetooth::SecurityOptions options);

protected:
    QBluetoothAbstractService *m_service;

private:
    QBluetoothServiceControlServer_Private *m_private;
};

#endif
