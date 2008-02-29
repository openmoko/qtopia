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

#ifndef _LOWMEMORYTASK_H_
#define _LOWMEMORYTASK_H_

#include "memorymonitor.h"
#include "applicationlauncher.h"

class LowMemoryTask : public QObject
{
Q_OBJECT
public:
    LowMemoryTask();
    virtual ~LowMemoryTask();

private slots:
    void memoryStateChanged(MemoryMonitor::MemState newState);
    void applicationStateChanged(const QString &, ApplicationTypeLauncher::ApplicationState);

private:
    QString *m_lastStartedApp;
};

#endif // _LOWMEMORYTASK_H_
