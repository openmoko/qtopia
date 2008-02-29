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

#include "applicationlauncher.h"
#include <QtopiaChannel>
#include <QtopiaServiceRequest>

// "calibrate" builtin
#include "../settings/calibrate/calibrate.h"
static QWidget *calibrate()
{
    Calibrate *c = new Calibrate;
    return c;
}
QTOPIA_SIMPLE_BUILTIN(calibrate, calibrate);

// "taskmanager" builtin
static QWidget *taskmanager()
{
    QtopiaServiceRequest( "TaskManager", "showRunningTasks()" ).send();
    return 0;
}
QTOPIA_SIMPLE_BUILTIN(taskmanager, taskmanager);

#ifdef QTOPIA_PHONEUI
// "callhistory" builtin
#ifdef QTOPIA_PHONE
#include <phone/homescreen.h>
static QWidget *callhistory()
{
    HomeScreen::getInstancePtr()->showCallHistory(false, QString());
    return 0;
}
QTOPIA_SIMPLE_BUILTIN(callhistory, callhistory);
#endif
#endif

// "shutdown" builtin
static QWidget *shutdown()
{
    QtopiaChannel::send( "QPE/System", "shutdown()" );
    return 0;
}
QTOPIA_SIMPLE_BUILTIN(shutdown, shutdown);

