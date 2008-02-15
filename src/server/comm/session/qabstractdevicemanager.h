/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef __QABSTRACTCOMMDEVICEMANAGER_H__
#define __QABSTRACTCOMMDEVICEMANAGER_H__

#include <qglobal.h>
#include <qobject.h>

class QString;
class QAbstractCommDeviceManager_Private;
class QUnixSocket;

// SERVER SIDE

class QAbstractCommDeviceManager : public QObject
{
    Q_OBJECT

    friend class QAbstractCommDeviceManager_Private;

public:
    QAbstractCommDeviceManager(const QByteArray &serverPath,
                               const QByteArray &devId,
                               QObject *parent = 0);
    virtual ~QAbstractCommDeviceManager();

    bool start();
    bool isStarted() const;
    void stop();

    // Assume UNIX domain socket implementation
    const QByteArray &serverPath() const;
    // Unique DeviceId this manager is handling
    const QByteArray &deviceId() const;

    bool sessionsActive() const;

protected:
    // Actual implementation of bringUp.  Needs to handle async bring up/down
    // situations.  On success up() signal should be sent, on error, error signal
    // should be sent
    virtual void bringUp() = 0;
    // same as above
    virtual void bringDown() = 0;

    // Should be here to handle the situation of manager being started and the device
    // already being in "on/off" position
    virtual bool isUp() const = 0;

    virtual bool shouldStartSession(QUnixSocket *socket) const;
    virtual bool shouldBringDown(QUnixSocket *socket) const;

signals:
    void upStatus(bool error, const QString &msg);
    void downStatus(bool error, const QString &msg);

private:
    QAbstractCommDeviceManager_Private *m_data;
};

#endif
