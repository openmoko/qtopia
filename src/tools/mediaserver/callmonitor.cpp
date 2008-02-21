/****************************************************************************
**
** Copyright (C) 2008-2008 TROLLTECH ASA. All rights reserved.
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

#include <QPhoneCallManager>

#include "callmonitor.h"

namespace mediaserver
{

class CallMonitorPrivate : public QObject
{
    Q_OBJECT

public:
    bool                active;
    QPhoneCallManager*  callManager;

signals:
    void callActivityChanged(bool active);

private slots:
    void checkCalls();
};

void CallMonitorPrivate::checkCalls()
{
    bool h = callManager->calls().count() > 0;

    if (active != h)
        emit callActivityChanged(active = h);
}


CallMonitor::CallMonitor(QObject* parent):
    QObject(parent),
    d(new CallMonitorPrivate)
{
    d->active = false;
    d->callManager = new QPhoneCallManager(this);
    connect(d->callManager, SIGNAL(statesChanged(QList<QPhoneCall>)), d, SLOT(checkCalls()));

    connect(d, SIGNAL(callActivityChanged(bool)), SIGNAL(callActivityChanged(bool)));
}

CallMonitor::~CallMonitor()
{
    delete d;
}

}   // ns mediaserver

#include "callmonitor.moc"


