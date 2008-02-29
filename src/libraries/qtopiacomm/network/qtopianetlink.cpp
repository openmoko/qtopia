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

#include "qtopianetlink.h"

//#include <asm/types.h>

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>
//#include <linux/rtnetlink.h>

#include <QSocketNotifier>
#include <QDebug>

#include <qtopialog.h>

#define UEVENT_BUFFER_SIZE 4096

class NetlinkPidProvider
{
public:
    NetlinkPidProvider()
        : pid( ::getpid() )
    {
    }

    int nextPid()
    {
        return pid++;
    }

private:
    int pid;
};

Q_GLOBAL_STATIC(NetlinkPidProvider, pidProvider );

bool QtopiaNetlink::supports( Protocol protocol )
{
    switch( protocol )
    {
        case Route:
#ifndef NETLINK_ROUTE
            return false;
#else
            return true;
#endif
            break;
        case KernelObjectEvent:
#ifndef NETLINK_KOBJECT_UEVENT
            return false;
#else
            return true;
#endif
            break;
    }
    return false;
}


class QtopiaNetlinkPrivate
{
public:
    QtopiaNetlinkPrivate( QtopiaNetlink::Protocols protocols )
        : protos( protocols )
    {
    };

    int initUEventSocket()
    {
#ifdef NETLINK_KOBJECT_UEVENT
        struct sockaddr_nl nl;
        memset( &nl, 0, sizeof( struct sockaddr_nl ) );
        nl.nl_family = AF_NETLINK;
        nl.nl_pid = pidProvider()->nextPid();
        nl.nl_groups = 1;

        int fd = socket( AF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT );
        if ( fd < 0 )
        {
            qLog(NetLink) << "Cannot open netlink uevent socket:" << strerror(errno) ;
            return -1;
        }

        int ret = ::bind( fd, (struct sockaddr *) &nl, 
                sizeof(struct sockaddr_nl) );
        if ( ret < 0 ) {
            qLog(NetLink) << "uevent socket bind failed:" << strerror(errno);
            ::close( fd );
            return -1;
        }

        return fd;
#else
        qWarning() << "Kernel doesn't support uevents";
        return -1;
#endif
    }

    void _q_readUEventMessage(int socket)
    {
#ifdef NETLINK_KOBJECT_UEVENT
        if ( socket < 0 )
            return;

        char buffer[UEVENT_BUFFER_SIZE];
        while ( 1 ) {
            ssize_t len = ::recv( socket, &buffer, sizeof(buffer), MSG_DONTWAIT );
            if ( len <= 0 ) { //error or peer shutdown
                break;
            }
            qLog(NetLink) << buffer;

        }
#endif
    }

    QtopiaNetlink::Protocols protos;

    QtopiaNetlink* q;
};


/*!
  \class QtopiaNetlink
  \internal
  \mainclass
  \brief The QtopiaNetlink provides direct access to kernel event notifications.

  \preliminary
  Warning: This class is work in progress. The API is subject to changes and WILL be changed
  in the future. 

  */

/*!
  \internal
  Constructs a new QtopiaNetlink instance with the given \a parent. \a protocols determines
  to what type of kernel events this object should listen to. 
  */
QtopiaNetlink::QtopiaNetlink( QtopiaNetlink::Protocols protocols, QObject* parent )
    : QObject( parent )
{
    d = new QtopiaNetlinkPrivate( protocols );
    d->q = this;
    if ( protocols & QtopiaNetlink::KernelObjectEvent ) 
    {
        int fd = d->initUEventSocket();
        if ( fd >= 0 ) {
            QSocketNotifier* notifier = new QSocketNotifier( fd, QSocketNotifier::Read );
            connect( notifier, SIGNAL(activated(int)), 
                    this, SLOT(_q_readUEventMessage(int)) );
        }
    }
}

/*!
  Destroys the QtopiaNetlink instance
  */
QtopiaNetlink::~QtopiaNetlink()
{
    delete d;
}



/*
signals:
    void routeNotification( QtopiaNetlink::Notification msg );
    void kernelUEventNotification( const QByteArray& message );

private:
    QtopiaNetlinkPrivate* d;
};

*/

#include "moc_qtopianetlink.cpp"
