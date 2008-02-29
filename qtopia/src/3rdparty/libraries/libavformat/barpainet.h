/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS and its licensors.
** All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See below for additional copyright and license information
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef BARPA_INET_H
#define BARPA_INET_H

#include "../config.h"

#ifdef CONFIG_BEOS_NETSERVER

# include <socket.h>
int inet_aton (const char * str, struct in_addr * add);
# define PF_INET AF_INET
# define SO_SNDBUF 0x40000001

/* fake */
struct ip_mreq {
    struct in_addr imr_multiaddr;  /* IP multicast address of group */
    struct in_addr imr_interface;  /* local IP address of interface */
};

#include <netdb.h>

#else
# include <arpa/inet.h>
#endif

#endif /* BARPA_INET_H */
