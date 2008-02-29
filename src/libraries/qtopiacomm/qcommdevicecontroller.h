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

#ifndef __QCOMMDEVICECONTROLLER_H__
#define __QCOMMDEVICECONTROLLER_H__

#include <qglobal.h>
#include <qobject.h>

#include <qtopiaglobal.h>

class QByteArray;

class QCommDeviceController_Private;

class QTOPIACOMM_EXPORT QCommDeviceController : public QObject
{
    Q_OBJECT

    friend class QCommDeviceController_Private;

public:
    enum PowerState { On, Off, OnOneItem, OnTimed };

    explicit QCommDeviceController(const QByteArray &deviceId, QObject *parent = 0);
    ~QCommDeviceController();

    const QByteArray &deviceId() const;

    void bringUp();
    void bringUpTimed(int secs);
    void bringUpOneItem();
    void bringDown();

    // This grabs the value of the device out of the QValueSpace
    bool isUp() const;

    PowerState powerState() const;

signals:
    // These are optional, but would be nice to know when the device is up/down for
    // settings apps.  Should be just a QValueSpace signal anyway.
    void up();
    void down();

    void powerStateChanged(QCommDeviceController::PowerState state);

private:
    QCommDeviceController_Private *m_data;
};

#endif
