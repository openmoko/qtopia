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

#ifndef _APPLICATIONMONITOR_H_
#define _APPLICATIONMONITOR_H_

#include <QObject>
#include <QPair>
#include <QList>
#include <QStringList>
#include "qtopiaserverapplication.h"
#include "applicationlauncher.h"

class UIApplicationMonitorPrivate;
class UIApplicationMonitor : public QObject
{
Q_OBJECT
public:
    UIApplicationMonitor(QObject *parent = 0);
    virtual ~UIApplicationMonitor();

    enum ApplicationState { NotRunning = 0x0000,
                            Starting = 0x0001,
                            Running  = 0x0002,

                            StateMask = 0x00FF,

                            NotResponding = 0x0100,
                            Busy = 0x0200,
                            Active = 0x0400 };

    ApplicationState applicationState(const QString &) const;

    QStringList runningApplications() const;
    QStringList notRespondingApplications() const;
    QStringList busyApplications() const;

    static int notRespondingTimeout();
    static void setNotRespondingTimeout(int);

signals:
    void applicationStateChanged(const QString &,
                                 UIApplicationMonitor::ApplicationState);
    void busy();
    void notBusy();

private:
    UIApplicationMonitorPrivate *d;
};

#endif // _APPLICATIONMONITOR_H_

