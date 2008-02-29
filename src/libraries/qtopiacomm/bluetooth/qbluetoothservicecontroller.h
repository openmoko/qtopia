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

#ifndef __QBLUETOOTHSERVICECONTROLLER_H__
#define __QBLUETOOTHSERVICECONTROLLER_H__

#include <QObject>
#include <QStringList>
#include <qbluetoothnamespace.h>

class QBluetoothServiceControllerPrivate;
class QTOPIACOMM_EXPORT QBluetoothServiceController : public QObject
{
    friend class QBluetoothServiceControllerPrivate;
    Q_OBJECT

public:
    enum ServiceState {
        NotRunning,
        Starting,
        Running
    };

    explicit QBluetoothServiceController(QObject *parent = 0);
    ~QBluetoothServiceController();

    void start(const QString &name);
    void stop(const QString &name);
    QBluetoothServiceController::ServiceState state(const QString &name) const;

    void setSecurityOptions(const QString &name,
                            QBluetooth::SecurityOptions options);
    QBluetooth::SecurityOptions securityOptions(const QString &name) const;

    QString displayName(const QString &name) const;

    QStringList services() const;

signals:
    void started(const QString &name, bool error, const QString &description);
    void stopped(const QString &name);

private:
    QBluetoothServiceControllerPrivate *m_data;
};

#endif
