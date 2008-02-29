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

#include "localsocket.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <qdebug.h>


/*!
  \class LocalSocket
  Very simple local socket class.

  The local socket is only able to connect to the well known path for the local
  socket, and send a string.

  In Unix, the local socket is a Unix stream socket.  In Windows or other OS's
  an equivalent construct, eg named pipe could be easily used (not implemented yet).
*/

/*!
  Construct a \c LocalSocket object
*/
LocalSocket::LocalSocket()
    : mDescriptor( -1 )
{
}

/*!
  Destruct a \c LocalSocket object
*/
LocalSocket::~LocalSocket()
{
    if ( mDescriptor != -1 )
        ::close( mDescriptor );
}

/*!
  Send the string \a request on the local socket.
*/
void LocalSocket::sendRequest( const QString &request )
{
    int resultCode;
    int nullBytes = 0;
    if ( mDescriptor == -1 )
        connect();
    if ( mDescriptor == -1 )
        return;
    resultCode = ::write( mDescriptor, (const char*)request.unicode(), request.length() * 2 );
    if ( resultCode < request.length() * 2 )
        qWarning() << "Failure during write" << strerror( errno ) << ": wrote" << resultCode << "bytes";
    resultCode = ::write( mDescriptor, &nullBytes, sizeof(int) );
}

void LocalSocket::connect()
{
    int resultCode;
    mDescriptor = ::socket(PF_UNIX, SOCK_STREAM, 0);
    if ( mDescriptor == -1 )
    {
        qWarning() << "Unable to create socket" << strerror(errno);
        return;
    }

    struct sockaddr_un localAddress;
    localAddress.sun_family = AF_UNIX;
    Q_ASSERT( strlen( LOCAL_SOCKET_PATH ) < sizeof( localAddress.sun_path ));
    ::memset( localAddress.sun_path, '\0', sizeof( localAddress.sun_path ));
    ::strcpy( localAddress.sun_path, LOCAL_SOCKET_PATH );

    resultCode = ::connect( mDescriptor, (sockaddr *)&localAddress, sizeof( localAddress ));
    if ( resultCode == -1 )
    {
        qWarning() << "Unable to connect" << strerror( errno );
        ::close( mDescriptor );
        mDescriptor = -1;
        return;
    }
}
