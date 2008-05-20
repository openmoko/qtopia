/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#include "applicationlauncher.h"
#include <QtopiaChannel>
#include <QtopiaServiceRequest>

// "taskmanager" builtin
static QWidget *taskmanager()
{
    QtopiaServiceRequest( "TaskManager", "showRunningTasks()" ).send();
    return 0;
}
QTOPIA_SIMPLE_BUILTIN(taskmanager, taskmanager);

#ifdef QTOPIA_PHONEUI
// "callhistory" builtin
#include <phone/homescreencontrol.h>
static QWidget *callhistory()
{
    if (HomeScreenControl::instance()->homeScreen())
        HomeScreenControl::instance()->homeScreen()->showCallHistory(false, QString());
    return 0;
}
QTOPIA_SIMPLE_BUILTIN(callhistory, callhistory);

static QWidget *dialer()
{
    if (HomeScreenControl::instance()->homeScreen())
        HomeScreenControl::instance()->homeScreen()->showDialer();
    return 0;
}
QTOPIA_SIMPLE_BUILTIN(dialer, dialer);

static QWidget *callScreen()
{
    // This will emit a signal
    if (HomeScreenControl::instance()->homeScreen())
        HomeScreenControl::instance()->homeScreen()->doShowCallScreen();
    return 0;
}
QTOPIA_SIMPLE_BUILTIN(callscreen, callScreen);
#endif

// "shutdown" builtin
static QWidget *shutdown()
{
    QtopiaChannel::send( "QPE/System", "shutdown()" );
    return 0;
}
QTOPIA_SIMPLE_BUILTIN(shutdown, shutdown);

#ifdef QTOPIA_CELL

// "simapp" builtin
#include "../applications/simapp/simapp.h"
static QWidget *simapp()
{
    SimApp *s = SimApp::instance();
    return s;
}
QTOPIA_SIMPLE_BUILTIN(simapp, simapp);

#endif

