/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef _TASKMANAGER_LAUNCHERVIEW_H_
#define _TASKMANAGER_LAUNCHERVIEW_H_


#include "launcherview.h"
#include "applicationmonitor.h"

#include <QString>
#include <QByteArray>
#include <QHash>


class QtopiaChannel;
class TaskManagerLauncherView : public LauncherView
{
Q_OBJECT
public:
    TaskManagerLauncherView(QWidget * = 0);
    ~TaskManagerLauncherView();

private slots:
    void applicationStateChanged();
    void receivedLauncherServiceMessage(const QString &msg, const QByteArray &args);
    void activatedHomeItem();

private:
    QString itemActivationIpcMessage(int itemId);
    void addDynamicLauncherItem(int id, const QString &name, const QString &iconPath);
    void removeDynamicLauncherItem(int id);

    static const QString LAUNCH_MSG_PREFIX;
    UIApplicationMonitor monitor;
    QtopiaChannel *m_channel;
    QHash<QString, QContent *> m_dynamicallyAddedItems;
};

#endif // _TASKMANAGER_LAUNCHERVIEW_H_
