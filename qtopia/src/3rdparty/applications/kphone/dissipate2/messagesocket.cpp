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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "messagesocket.h"

MessageSocket::MessageSocket( void )
{
	type = None;
	bound = false;
}

MessageSocket::~MessageSocket( void )
{
}

bool MessageSocket::setHostname( const char *hostname )
{
	if ( ( he = gethostbyname( hostname ) ) == NULL ) {
		perror( "MessageSocket::setHostname(): gethostbyname() failed" );
		return false;
	}

	return true;
}

void MessageSocket::forcePortNumber( unsigned int newport )
{
	ourport = newport;
}



