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

#ifndef QTOPIA_NETLINK_H
#define QTOPIA_NETLINK_H
#include <QObject>

#include <qtopiaglobal.h>

class QtopiaNetlinkPrivate;
class QTOPIACOMM_EXPORT QtopiaNetlink : public QObject
{
    Q_OBJECT
public:
    
    enum Protocol
    {
        Route = 0x0001,      //see rtnetlink(7), rtnetlink(3)
        KernelObjectEvent = 0x0002
    };

    Q_DECLARE_FLAGS(Protocols, Protocol);

    enum RouteNotification
    {
        NewAddress = 0,
        DelAddress = 1,

        NewRoute = 99,
        DelRoute = 100,
    };


    QtopiaNetlink( QtopiaNetlink::Protocols protocol, QObject* parent = 0 );
    ~QtopiaNetlink();

    static bool supports( Protocol protocol );
signals:
    void routeNotification( QtopiaNetlink::RouteNotification msg );
    void kernelUEventNotification( const QByteArray& message );

private:
    Q_PRIVATE_SLOT( d, void _q_readUEventMessage(int) );
private:
    QtopiaNetlinkPrivate* d;
};

#endif //QTOPIA_NETLINK_H
