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

#ifndef __QTOPIA_MEDIASERVER_CALLMONITOR_H
#define __QTOPIA_MEDIASERVER_CALLMONITOR_H

#include <QObject>

namespace mediaserver
{

class CallMonitorPrivate;

class CallMonitor : public QObject
{
    Q_OBJECT

public:
    CallMonitor(QObject* parent = 0);
    ~CallMonitor();

signals:
    void callActivityChanged(bool active);

private:
    CallMonitorPrivate* d;
};

}   // ns mediaserver

#endif  // __QTOPIA_MEDIASERVER_CALLMONITOR_H
