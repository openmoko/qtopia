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

#ifndef __DBUSROUTER_H__
#define __DBUSROUTER_H__

#if defined(QTOPIA_DBUS_IPC)
#include "applicationlauncher.h"

class DBUSQtopiaApplicationChannel;

class DBusRouter : public ApplicationIpcRouter
{
    Q_OBJECT
public:
    DBusRouter();
    ~DBusRouter();

    virtual void addRoute(const QString &app, RouteDestination *);
    virtual void remRoute(const QString &app, RouteDestination *);

private slots:
    void applicationMessage( const QString& msg, const QByteArray& data );

private:
    void routeMessage(const QString &, const QString &, const QByteArray &);
    QMultiMap<QString, RouteDestination *> m_routes;
    QMap<QString, DBUSQtopiaApplicationChannel *> m_channels;
};

#endif

#endif // QCOPROUTER_H
