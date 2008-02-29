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

#include "qtopiaserverapplication.h"
#include <QTimer>

class ApplicationLauncher;
class ApplicationShutdownTask : public SystemShutdownHandler
{
Q_OBJECT
public:
    ApplicationShutdownTask();

    virtual bool systemRestart();
    virtual bool systemShutdown();

private slots:
    void terminated();
    void timeout();

private:
    bool doShutdown();
    bool allAppsQuit();
    void killAll();

    enum State { NoShutdown, WaitingForShutdown, KilledApps, Shutdown };
    State m_state;
    QTimer m_timer;
    ApplicationLauncher *m_launcher;
};

