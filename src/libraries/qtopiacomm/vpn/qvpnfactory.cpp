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

#include "qvpnfactory.h"

#include "qopenvpn_p.h"
#include "qipsec_p.h"

#include <QDebug>
#include <QDir>
#include <qtopialog.h>
#include <qtopianamespace.h>

/*!
  \class QVPNFactory
  \mainclass
  \brief The QVPNFactory class creates QVPNClient instances.

  \ingroup io

  The virtual private network factory provides access to QVPNClient instances.
  create() should be used when a new VPN connection is required and instance() 
  should be used to obtain a reference to an already existing VPN connection.
  
  In addition QVPNFactory provides functionality to lookup and resolve VPN identifier.
  vpnIDs() returns a list of all known virtual private network connections known to Qtopia
  and name() returns the human-readable name of a given connection.

  QVPNFactory follows the factory pattern and creates QVPNClient instances. types() should
  be used to determine what types of VPN's are supported by a particular build of Qtopia.

  \sa QVPNClient

*/

/*!
  Constructs a factory for virtual private network objects.
  */
QVPNFactory::QVPNFactory()
    : serverMode( false )
{
}


/*!
  Instanciates a QVPNClient object for the virtual private network
  specified by \a vpnID with the given \a parent. If no VPN with \a vpnID
  exists this function returns a null pointer.
*/
QVPNClient* QVPNFactory::instance( uint vpnID,  QObject* parent )
{
    QVPNClient* result = 0;

    const QString path = Qtopia::applicationFileName( "Network", "vpn" );
    QDir dir(path);
    dir.mkdir( path );

    const QStringList files = dir.entryList( QStringList("*.conf") );
    bool foundVPN = false;
    QVPNClient::Type type = QVPNClient::OpenVPN;
    QString file;
    foreach( QString entry, files ) {
        file = dir.filePath(entry);
        if ( qHash(file) == vpnID ) {
            QSettings cfg( file, QSettings::IniFormat );
            QVariant v = cfg.value("Info/Type");
            if ( v.isValid() ) {
                type = (QVPNClient::Type) v.toInt();
                foundVPN = true;
                break;
            }
        }
    }

    if ( foundVPN ) {
#ifndef QTOPIA_NO_OPENVPN
        if ( type == QVPNClient::OpenVPN )
            result = new QOpenVPN( serverMode, vpnID, parent );
#endif
#ifndef QTOPIA_NO_IPSEC
        if ( type == QVPNClient::IPSec )
            result = new QIPSec( serverMode, vpnID, parent );
#endif
    }

    return result;
}

/*!
  Requests a new virtual private network of \a type with the given \a parent.
  The returned VPN is a newly created and requires detailed configuration.
  If \a type is unknown this function returns a null pointer.
  */
QVPNClient* QVPNFactory::create( QVPNClient::Type type, QObject* parent )
{
    QVPNClient* result = 0;
#ifndef QTOPIA_NO_OPENVPN
    if ( type == QVPNClient::OpenVPN )
        result = new QOpenVPN( parent );
#endif
#ifndef QTOPIA_NO_IPSEC
    if ( type == QVPNClient::IPSec )
        result = new QIPSec( parent );
#endif
    return result;
}

/*!
  Returns a list of all possible/supported virtual private network types.
  */
QSet<QVPNClient::Type> QVPNFactory::types()
{
    QSet<QVPNClient::Type> result;
#ifndef QTOPIA_NO_OPENVPN
    result.insert( QVPNClient::OpenVPN );
#endif
#ifndef QTOPIA_NO_IPSEC
    //TODO enable IPSec
    //result.insert( QVPNClient::IPSec );
#endif
    return result;
}

/*!
  Returns the user-visible name of the VPN specified by \a vpnID. The function
  returns an empty string if vpnID is not a valid id.
  */
QString QVPNFactory::name( uint vpnID )
{
    const QString path = Qtopia::applicationFileName( "Network", "vpn" );
    QDir dir(path);
    dir.mkdir( path );

    const QStringList files = dir.entryList( QStringList("*.conf") );
    QString file;
    foreach( QString entry, files ) {
        file = dir.filePath(entry);
        if ( qHash(file) == vpnID ) {
            QSettings cfg( dir.filePath(entry), QSettings::IniFormat );
            return cfg.value("Info/Name").toString();
        }
    }

    return QString();
}

/*!
  Returns a list of all known virtual private networks.
  */
QSet<uint> QVPNFactory::vpnIDs()
{
    QSet<uint> result;
    const QString path = Qtopia::applicationFileName( "Network", "vpn" );
    QDir dir( path );
    dir.mkdir( path );

    const QStringList files = dir.entryList(QStringList("*.conf"));
    foreach( QString entry, files ) {
        result.insert( qHash(dir.filePath(entry)) );
    }
    return result;
}

/*!
  \internal
  The QtopiaVPNManager is the only instance that actually starts and stops a virtual private network.
  Any QVPNClient which is created in non-server mode forwards start/stop requests to the QVPNManager.
  */
void QVPNFactory::setServerMode( bool enable )
{
    serverMode = enable;
}

