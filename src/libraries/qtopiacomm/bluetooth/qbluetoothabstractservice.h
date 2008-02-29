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

#ifndef __QBLUETOOTHABSTRACTSERVICE_H__
#define __QBLUETOOTHABSTRACTSERVICE_H__

#include <qabstractipcinterfacegroup.h>
#include <qbluetoothnamespace.h>

class QBluetoothAddress;
class QBluetoothAbstractService_Private;

class QTOPIACOMM_EXPORT QBluetoothAbstractService
            : public QAbstractIpcInterfaceGroup
{
    friend class QBluetoothAbstractService_Private;
    Q_OBJECT

public:
    explicit QBluetoothAbstractService(const QString &name, QObject *parent = 0);
    virtual ~QBluetoothAbstractService();

    virtual void initialize();

    virtual void start(int channel) = 0;
    virtual void stop() = 0;
    virtual void setSecurityOptions(QBluetooth::SecurityOptions options) = 0;

    virtual QString translatableDisplayName() const = 0;


protected:
    bool sdpRegister(const QBluetoothAddress &local,
                     QBluetooth::SDPProfile profile,
                     int channel);
    bool sdpUnregister();
    QString sdpError();

private:
    QBluetoothAbstractService_Private *m_private;

signals:
    void started(QBluetooth::ServiceError error, const QString &errorDesc);
    void stopped(QBluetooth::ServiceError error, const QString &errorDesc);
    void error(QBluetooth::ServiceError error, const QString &errorDesc);
};

#endif
