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

#ifndef _DEVICEBUTTONTASK_H_
#define _DEVICEBUTTONTASK_H_

#include <qwindowsystem_qws.h>
#include "qtopiaserverapplication.h"
class QDeviceButton;
class PressHoldGate;
class QValueSpaceItem;
class DeviceButtonTask : public QObject, public QWSServer::KeyboardFilter
{
Q_OBJECT
public:
    DeviceButtonTask();

    virtual bool filter(int unicode, int keycode, int modifiers, bool press,
                        bool autoRepeat);

signals:
    void activated(int, bool);

private slots:
    void doActivate(int, bool);

private:
    bool keyLocked();

    QValueSpaceItem* vs;
    PressHoldGate*   ph;
};

QTOPIA_TASK_INTERFACE(DeviceButtonTask);
#endif // _DEVICEBUTTONTASK_H_
