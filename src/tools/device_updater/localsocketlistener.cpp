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

#include "localsocketlistener.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>

#include <QCoreApplication>
#include <QSocketNotifier>
#include <QFile>
#include <QDir>
#include <QTimer>

#include <qdebug.h>

/*!
  \class LocalSocketListener
  \brief Very simple local socket server

  All this class can do is listen on a local socket, and when a message
  is received send it via a Qt signal.

  Its assumed that a client only sends one message, and then drops its
  connection and exits.

  In Unix, the local socket is implemented as a Unix stream socket.  In
  other OS's a similar construct should be used, eg a named pipe.
  */

/*!
  Construct a new \c LocalSocketListener
  */
LocalSocketListener::LocalSocketListener()
    : mNotifier( 0 )
{
}

/*!
  Destroy a \c LocalSocketListener
  */
LocalSocketListener::~LocalSocketListener()
{
    if ( mNotifier )
        delete mNotifier;
    if ( QFile::exists( LOCAL_SOCKET_PATH ))
        QFile::remove( LOCAL_SOCKET_PATH );
}

/*!
  Listen for an incoming message.  If setting up for listen was
  successful, then the method isErrorCondition() will return false
  */
bool LocalSocketListener::listen()
{
    int descriptor;
    int resultCode;

    if ( QFile::exists( LOCAL_SOCKET_PATH ))
    {
        qWarning() << LOCAL_SOCKET_PATH " found: program already running"
            << "passing any commands to running instance";
        return false;
    }

    descriptor = ::socket( PF_UNIX, SOCK_STREAM, 0 );
    setDescriptor( descriptor );
    if( descriptor == -1 )
    {
        qWarning() << "Unable to create socket" <<
            QDir::current().filePath( LOCAL_SOCKET_PATH ) << strerror(errno);
        return false;
    }

    struct sockaddr_un localAddress;
    localAddress.sun_family = AF_UNIX;
    Q_ASSERT( strlen( LOCAL_SOCKET_PATH ) < sizeof( localAddress.sun_path ));
    ::memset( localAddress.sun_path, '\0', sizeof( localAddress.sun_path ));
    ::memcpy( localAddress.sun_path, LOCAL_SOCKET_PATH, strlen( LOCAL_SOCKET_PATH ));

    resultCode = ::bind( descriptor, (sockaddr *)&localAddress, sizeof( localAddress ));
    if ( resultCode == -1 )
    {
        qWarning() << "Unable to bind socket:" << strerror(errno);
        ::close( descriptor );
        setDescriptor( -1 );
        return false;
    }

    resultCode = ::listen( descriptor, MAX_CONNECTIONS );
    if ( resultCode == -1 )
    {
        qWarning() << "Unable to listen on socket:" << strerror(errno);
        close( descriptor );
        setDescriptor( -1 );
        return false;
    }
    qDebug() << "Listening on local socket" << LOCAL_SOCKET_PATH;
    return true;
}

void LocalSocketListener::setupNotifier()
{
    mNotifier = new QSocketNotifier( descriptor(), QSocketNotifier::Read, this );
    mNotifier->setEnabled( true );
    QObject::connect( mNotifier, SIGNAL(activated(int)), this, SLOT(receiveMessage()));
}

/*!
  \internal
  Slot to receive QSocketNotifier activation from data arriving on the socket
  */
void LocalSocketListener::receiveMessage()
{
    int resultCode;
    char msg[1024];
    QByteArray bytes;
    int clientDescriptor;
    struct sockaddr_un clientAddress;
    socklen_t addrSize = sizeof(clientAddress);

    clientDescriptor = ::accept( descriptor(), (struct sockaddr*)&clientAddress, &addrSize );
    if ( clientDescriptor == -1 )
    {
        qWarning() << "Could not accept connection" << strerror( errno );
        return;
    }

    ::memset( msg, '\0', 1024 );
    while (( resultCode = ::read( clientDescriptor, msg, 1023 )))
    {
        bytes.append( msg );
        ::memset( msg, '\0', 1024 );
    }
    ::close( clientDescriptor );
    QString command( (QChar*)bytes.constData(), bytes.size() / 2 );
    emit commandReceived( command );
}
