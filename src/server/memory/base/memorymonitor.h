// -*-C++-*-
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

#ifndef _MEMORYMONITOR_H_
#define _MEMORYMONITOR_H_

#include <QObject>
#include "qtopiaserverapplication.h"

class MemoryMonitor : public QObject
{
    Q_OBJECT

  public:
    enum MemState {
        MemUnknown      = 0,
        MemCritical     = 1,
        MemVeryLow      = 2,
        MemLow          = 3,
        MemNormal       = 4
    };

    virtual MemState memoryState() const = 0;
    virtual unsigned int timeInState() const = 0;

  signals:
    void memoryStateChanged(MemoryMonitor::MemState newState);
};

QTOPIA_TASK_INTERFACE(MemoryMonitor);

#endif // _MEMORYMONITOR_H_
