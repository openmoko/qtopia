/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef _SYSTEMSUSPEND_H_
#define _SYSTEMSUSPEND_H_

#include <QObject>
#include <QtopiaAbstractService>
#include "qtopiaserverapplication.h"

class SystemSuspendHandler : public QObject
{
Q_OBJECT
public:
    SystemSuspendHandler(QObject *parent = 0)
        : QObject(parent) {}

    virtual bool canSuspend() const = 0;
    virtual bool suspend() = 0;
    virtual bool wake() = 0;

signals:
    void operationCompleted();
};
QTOPIA_TASK_INTERFACE(SystemSuspendHandler);

class SystemSuspend : public QObject
{
Q_OBJECT
public:
    SystemSuspend(QObject *parent = 0) : QObject(parent) {}

public slots:
    virtual bool suspendSystem() = 0;

signals:
    void systemSuspending();
    void systemWaking();
    void systemActive();
    void systemSuspendCanceled();
};
QTOPIA_TASK_INTERFACE(SystemSuspend);

class SuspendService : public QtopiaAbstractService
{
Q_OBJECT
public:
    SuspendService(QObject *parent);

signals:
    void doSuspend();

public slots:
    void suspend();
};


#endif // _SYSTEMSUSPEND_H_

