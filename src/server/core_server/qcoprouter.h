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

#ifndef QCOPROUTER_H
#define QCOPROUTER_H

#if !defined(QTOPIA_DBUS_IPC) && !defined(QT_NO_COP)

#include <QMultiMap>
#include "applicationlauncher.h"
#include <QSet>
#ifdef Q_WS_X11
#include <qcopchannel_x11.h>
#else
#include <qcopchannel_qws.h>
#endif

class QCopRouter : public ApplicationIpcRouter
{
    Q_OBJECT
public:
    QCopRouter();
    ~QCopRouter();

    virtual void addRoute(const QString &app, RouteDestination *);
    virtual void remRoute(const QString &app, RouteDestination *);

private slots:
    void applicationMessage( const QString& msg, const QByteArray& data );
    void serviceMessage( const QString& msg, const QByteArray& data );

private:
    void routeMessage(const QString &, const QString &, const QByteArray &);
    QMultiMap<QString, RouteDestination *> m_routes;

    // In-progress route
    QString m_cDest;
    QString m_cMessage;
    QByteArray m_cData;
    QSet<RouteDestination *> m_cRouted;
};

#endif

#endif // QCOPROUTER_H
