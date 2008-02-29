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

#include "qipsec_p.h"
#include "qvpnclientprivate_p.h"

#ifndef QTOPIA_NO_IPSEC
/*!
  \internal
  \class QIPSec
  \internal This class represents an OpenVPN network.

  An QIPSec objects operates in two distinct modes.
 */

/*!
  \internal

  Constructs a virtual private OpenVPN network with the given \a parent.
  The server mode is used bei vpnmanager only and indicates that this object
  should actually start the vpn connection. An OpenVPN object that is not
  running in server mode will forward connect() and disconnect() calls to the server.
*/
QIPSec::QIPSec( QObject* parent )
    : QVPNClient( false, parent )
{
}

/*!
  \internal

  \a vpnID allows the slection of a particular IPSec connection.
  */
QIPSec::QIPSec( bool serverMode, uint vpnID, QObject* parent )
    : QVPNClient( serverMode, vpnID, parent )
{
}

/*!
  \internal
  Destroys the virtual private network.
*/
QIPSec::~QIPSec()
{
}

/*!
  \internal
  Returns QVPNClient::IPSec.
  */
QVPNClient::Type QIPSec::type() const
{
    return QVPNClient::IPSec;
}

/*!
  Starts a new IPSec connection.
  */
void QIPSec::connect()
{
}

/*!
  \internal
  Disconnects the network.
  */
void QIPSec::disconnect()
{
}

/*!
  \internal
  Returns a configuration dialog for this IPSec network. The caller is responsible
  for the deletion of the returned pointer.
  */
QDialog* QIPSec::configure( QWidget* parent )
{
    //TODO
    return 0;
}

/*!
  \internal
  Returns the human-readable name of this OpenVPN connection.
  */
QString QIPSec::name() const
{
    if ( d->config.isEmpty() )
        return tr("IPSec connection");
    return QVPNClient::name();
}

/*!
  \internal
  Returns the current state of the network.
  */
QVPNClient::State QIPSec::state() const
{
    //TODO
    return QVPNClient::Disconnected;
}

/*!
  \internal
  Deletes the IPSec connection and all files associated to it.
  \warning This function does nothing if the connection is still active.
  */
void QIPSec::cleanup()
{
    if ( state() != Disconnected )
        return;
}
#endif //QTOPIA_NO_IPSEC
