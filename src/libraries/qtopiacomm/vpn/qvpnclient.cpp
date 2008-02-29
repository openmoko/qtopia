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

#include "qvpnclient.h"
#include "qvpnclientprivate_p.h"

#include <QHash>
#include <QProcess>

#include <qtopianamespace.h>
#include <qtopialog.h>

/*!
  \class QVPNClient
  \brief The QVPNClient class abstracts data and state of a virtual private network.

  An instance of a QVPNClient can be created by using a QVPNFactory. New VPN implementations
  must subclass this abstract class.

  For more details see \l QVPNFactory::create()

  \ingroup io
*/

/*!
  \enum QVPNClient::Type

  This enum is used to describe the type of a VPN connection.

  \value OpenVPN VPN solution based on OpenVPN (for details see \l http://openvpn.net)
  \value IPSec VPN solution based on IPSec (not yet implemented in
                Qtopia and serves as place holder for future implementation)
*/

/*!
  \enum QVPNClient::State

  This enum is used to describe the state of a VPN connection.

  \value Disconnected The VPN connection is offline.
  \value Pending The VPN connection is in a transitional phase between Disconnected and Connected.
  \value Connected The VPN connection is active.
*/



/*!
  \internal
  */
QVPNClient::QVPNClient( bool serverMode, QObject* parent )
    : QObject( parent )
{
    d = new QVPNClientPrivate();
    d->serverMode = serverMode;

}

/*!
  \internal
  */
QVPNClient::QVPNClient( bool serverMode, uint vpnID, QObject* parent )
    : QObject( parent )
{
    d = new QVPNClientPrivate();
    d->serverMode = serverMode;

    const QString path = Qtopia::applicationFileName( "Network", "vpn" );
    QDir dir(path);
    dir.mkdir( path );

    const QStringList files = dir.entryList( QStringList("*.conf") );
    QString file;
    foreach( QString entry, files ) {
        file = dir.filePath(entry);
        if ( qHash(file) == vpnID ) {
            d->config = file;
            break;
        }
    }
    if ( qLogEnabled(VPN) && d->config.isEmpty() )
        qLog(VPN) << "Unknown VPN id: "<< vpnID;
}

/*!
  Destroys the virtual private network.
*/
QVPNClient::~QVPNClient()
{
    delete d;
    d = 0;
}

/*!
  Returns the unique ID of this virtual private network.
*/
uint QVPNClient::id() const
{
    return qHash( d->config );
}

/*!
  Returns the user set name of this connection.
  */
QString QVPNClient::name() const
{
    QSettings cfg( d->config, QSettings::IniFormat );
    return cfg.value( "Info/Name", QString() ).toString();
}

/*!
  Returns a human-readable description of the last error that occurred. This is useful for
  presenting an error message to the user when receiving a connectionStateChanged() signal
  with the error argument set to true;
  */
QString QVPNClient::errorString() const
{
    return d->errorString;
}

/*!
  \fn void QVPNClient::connect()

  This function starts the VPN connection.
  */

/*!
  \fn void QVPNClient::disconnect()

  This function stops the active connection.
  */

/*!
  \fn QDialog* QVPNClient::configure( QWidget* parent )

  Returns the default configuration dialog that is shown to the user
  when he configures this interface. \a parent is the parent widget for
  the returned dialog.
  */

/*!
  \fn State QVPNClient::state() const

  Returns the current state of this VPN client.

  \sa QVPNClient::State
  */

/*!
  \fn Type QVPNClient::type() const

  Returns the VPN client type.

  \sa QVPNClient::Type
  */

/*!
  \fn void QVPNClient::cleanup()

  Deletes the VPN client an all files associated to it. This function
  does nothing if the client is still active.
  */

/*!
  \fn void QVPNClient::connectionStateChanged( bool error )

  This signal is emitted when the state of the VPN connetion changes.
  \a error will be set to \c TRUE if an error occured during the last state transition.
  */
