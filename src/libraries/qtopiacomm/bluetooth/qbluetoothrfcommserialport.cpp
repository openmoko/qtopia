/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include "qbluetoothrfcommserialport.h"
#include "qbluetoothnamespace_p.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

#include <QDebug>

#include <qtopialog.h>

class QBluetoothRfcommSerialPortPrivate: public QObject
{
public:
    QBluetoothRfcommSerialPortPrivate( QObject* parent = 0 )
        : QObject( parent ), deviceID( -1 )
    {
    }

    int16_t deviceID;
};


/*!
  \class QBluetoothRfcommSerialPort
  \brief The QBluetoothRfcommSerialPort class represents a serial device which enables access to a bluetooth serial port protocol

  This class provides the same functionality as the command line tool rfcomm.
  It allows to set up and maintain the RFCOMM configuration of the Bluetooth subsystem.

  \ingroup qtopiabluetooth
*/

/*!
  Constrcuts a new RFCOMM serial port object.  The \a parent is passed to the QObject constructor.
  */
QBluetoothRfcommSerialPort::QBluetoothRfcommSerialPort( QObject* parent)
    : QObject( parent )
{
    d = new QBluetoothRfcommSerialPortPrivate( this );
}

/*!
  Destroys a Bluetooth serial port object. If this object (RFCOMM device) is bound and/or connected
  to a remote Bluetooth device the associated link will automatically be removed/released.
  */
QBluetoothRfcommSerialPort::~QBluetoothRfcommSerialPort()
{
    if ( d->deviceID >= 0 )
        releaseTty();
}

/*!
  Binds \a socket to an RFCOMM device and closes \a socket in the process. The return value is the name of the
  RFCOMM device (typically /dev/rfcommXXX). If this function call fails,
  the return value is an empty string.

  Note: The caller should call QBluetoothRfcommSocket::close immediately after calling
        this function.  Before the created QBluetoothRfcommSerialPort device can be used,
        the \a socket should be closed.
  */
QString QBluetoothRfcommSerialPort::createTty( QBluetoothRfcommSocket* socket )
{
    if ( d->deviceID >= 0 ) //we are bound to a tty interface already
        return QString();

    struct rfcomm_dev_req rqst;
    ::memset( &rqst, 0, sizeof( struct rfcomm_dev_req ) );
    str2bdaddr( socket->remoteAddress().toString(), &rqst.dst );
    str2bdaddr( socket->localAddress().toString(), &rqst.src );
    rqst.dev_id = -1;
    rqst.flags = 0;

    struct sockaddr_rc saddr;
    socklen_t len = sizeof( saddr );

    if ( getsockname( socket->socketDescriptor(), (struct sockaddr*)&saddr, &len ) < 0 )
        return QString();
    rqst.channel = saddr.rc_channel;
    rqst.flags = (1 << RFCOMM_REUSE_DLC ) |( 1 << RFCOMM_RELEASE_ONHUP );

    // Set the socket non-blocking
    int flags = fcntl(socket->socketDescriptor(), F_GETFL, 0);
    flags &= ~O_NONBLOCK;
    fcntl(socket->socketDescriptor(), F_SETFL, flags);

    d->deviceID = ioctl( socket->socketDescriptor(), RFCOMMCREATEDEV, &rqst );
    if ( d->deviceID < 0 ) {
        qLog(Bluetooth) << "RfcommSerialPort::createTty( socket ): " << strerror(errno);
        return QString();
    }

    return QString("/dev/rfcomm"+QString::number(d->deviceID));
}

/*!
  Binds an RFCOMM device to the remote Bluetooth device with the address \a remote on \a channel.
  The local device given by \a local is used to open the connection.  Please note that calling this function
  will not established a connection to the remote Bluetooth device. The connection is established as soon
  as an application opens the device. The return value is the name of the newly created RFCOMM device (typically /dev/rfcommXX).
  If this function call fails, the return value is an empty string.
*/
QString QBluetoothRfcommSerialPort::createTty( const QBluetoothAddress& local,
        const QBluetoothAddress& remote, int channel)
{
    if ( d->deviceID >= 0 || !local.valid() || !remote.valid() ) //we are bound to a tty interface already
        return QString();

    int socket = ::socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_RFCOMM );
    if ( socket < 0 ) {
        perror("QBluetoothRfcommSerialPort::bind");
        return QString();
    }

    struct rfcomm_dev_req rqst;
    ::memset( &rqst, 0, sizeof( struct rfcomm_dev_req ) );
    str2bdaddr( remote.toString(), &rqst.dst );
    str2bdaddr( local.toString(), &rqst.src );
    rqst.dev_id = -1;
    rqst.flags = 0;
    rqst.channel = channel;

    d->deviceID = ::ioctl( socket, RFCOMMCREATEDEV, &rqst );
    close( socket );
    if ( d->deviceID < 0 ) {
        perror( "QBluetoothRfcommSerialPort::bind");
        return QString();
    }
    return QString("/dev/rfcomm"+QString::number(d->deviceID));
}

/*!
  This function releases an RFCOMM device established via bind().
  */
void QBluetoothRfcommSerialPort::releaseTty()
{
    int socket = ::socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_RFCOMM );
    if ( socket < 0 ) {
        perror("QBluetoothRfcommSerialPort::release");
        return;
    }

    struct rfcomm_dev_req rqst;
    ::memset( &rqst, 0, sizeof( struct rfcomm_dev_req ) );
    rqst.dev_id = d->deviceID;
    rqst.flags = (1 << RFCOMM_HANGUP_NOW );
    int ret = ::ioctl( socket, RFCOMMRELEASEDEV, &rqst );
    if ( ret < 0  && errno != ENODEV )
        perror( "QBluetoothRfcommSerialPort::release" );
    else
        d->deviceID = -1;

    ::close( socket );

    return;
}

/*!
  Returns a list of all RFCOMM device bindings.
   */
QStringList QBluetoothRfcommSerialPort::listBindings()
{
    QStringList result;
    int s = socket( AF_BLUETOOTH, SOCK_RAW, BTPROTO_RFCOMM );
    if ( s < 0 ) {
        perror("QBluetoothRfcommSerialPort::listBindings");
        return result;
    }

    struct rfcomm_dev_info* info;
    struct rfcomm_dev_list_req* allInfos;

    allInfos = (struct rfcomm_dev_list_req* ) malloc( RFCOMM_MAX_DEV*sizeof(struct rfcomm_dev_info) + sizeof(struct rfcomm_dev_list_req));
    allInfos->dev_num = RFCOMM_MAX_DEV;
    info = allInfos->dev_info;

    int ret = ::ioctl( s, RFCOMMGETDEVLIST, allInfos );
    if  ( ret < 0 ) {
        perror("QBluetoothRfcommSerialPort::listBindings: Cannot obtain list of rfcomm devices");
    } else {
        struct rfcomm_dev_info* next;
        for( int i = 0; i<allInfos->dev_num; i++ ) {
            next = info+i;
            QString dev( "/dev/rfcomm/" + QString::number( next->id ) );
            result.append( dev );
        }
    }
    free( allInfos );

    close( s );
    return result;
}

/*!
  Returns the RFCOMM device on which the current instance of this class is operating
  otherwise an empty string.
  */
QString QBluetoothRfcommSerialPort::boundDevice() const
{
    if ( d->deviceID < 0 )
        return QString();

    return QString( "/dev/rfcomm"+QString::number( d->deviceID ) );
}


