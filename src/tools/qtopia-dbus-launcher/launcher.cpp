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

#include "launcher.h"

#include <qtdbus/qdbusconnection.h>
#include <qtdbus/qdbusmessage.h>

#include <QCoreApplication>
#include <QDebug>

#include <stdio.h>
#include <stdlib.h>

Launcher::Launcher(QObject *parent) : QObject(parent)
{
}

Launcher::~Launcher()
{

}

void Launcher::startup(const QString &app)
{
    QDBusConnection dbc = QDBus::sessionBus();

    if (!dbc.isConnected()) {
        fprintf(stderr, "QtopiaLauncher: Could not connect to DBUS bus\n");
        exit(1);
    }

    bool r = dbc.connect(QString(),                           // Service
                         "/DBusLauncher",                     // Path
                         "com.trolltech.qtopia.DBusLauncher", // Interface
                         "launched",                          // Name
                         this,
                         SLOT(handleSignalReceived(QString,QDBusMessage)));

    QDBusMessage message =
            QDBusMessage::methodCall("com.trolltech.qtopia.DBusLauncher",
                                     "/DBusLauncher",
                                     "com.trolltech.qtopia.DBusLauncher",
                                     "launch", dbc);
    message << app;

    bool ret = dbc.send(message);
    if (!ret)
        fprintf(stderr, "Unable to send message: %s", dbc.lastError().message().toAscii().constData());

    m_app = app;
}

void Launcher::handleSignalReceived(const QString &app, const QDBusMessage &msg)
{
    if (app == m_app) {
        QCoreApplication::quit();
    }
}
