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

#ifndef __QBLUETOOTHSERVICECONTROLLER_H__
#define __QBLUETOOTHSERVICECONTROLLER_H__

#include <QObject>

#include <qbluetoothnamespace.h>

class QBluetoothServiceController_Private;
class QTOPIACOMM_EXPORT QBluetoothServiceController : public QObject
{
    friend class QBluetoothServiceController_Private;
    Q_OBJECT

public:
    enum ServiceState {
        Stopped,
        Started,
        Stopping,
        Starting
    };

    explicit QBluetoothServiceController(QObject *parent = 0);
    ~QBluetoothServiceController();

    void start(const QString &name);
    void stop(const QString &name);
    QBluetoothServiceController::ServiceState state(const QString &name) const;

    void setSecurityOptions(const QString &name,
                            QBluetooth::SecurityOptions options);
    QBluetooth::SecurityOptions securityOptions(const QString &name) const;

    QString translatableDisplayName(const QString &name) const;

    QList<QString> registeredServices() const;
    bool isRegistered(const QString &name) const;

signals:
    void started(const QString &name,
                 QBluetooth::ServiceError error,
                 const QString &errorDesc);
    void stopped(const QString &name,
                 QBluetooth::ServiceError error,
                 const QString &errorDesc);
    void error(const QString &name,
               QBluetooth::ServiceError error,
               const QString &errorDesc);

private:
    QBluetoothServiceController_Private *m_private;
};

#endif
