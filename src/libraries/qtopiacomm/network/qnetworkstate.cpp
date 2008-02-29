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

#include "qnetworkstate.h"

#include <QHash>
#include <QSettings>
#include <QStringList>

#include <qtopianetwork.h>
#include <qnetworkdevice.h>
#include <qvaluespace.h>

class QNetworkStatePrivate : public QObject
{
    Q_OBJECT
public:
    QNetworkStatePrivate( QObject* parent )
        : QObject( parent )
    {
        netSpace = new QValueSpaceItem( "/Network/Interfaces", this );
        update();
        connect( netSpace, SIGNAL(contentsChanged()), this, SLOT(update()) );
        gwSpace = new QValueSpaceItem( "/Network/Gateway" , this );
        connect( gwSpace, SIGNAL(contentsChanged()), this, SLOT(gatewayChanged()) );
    }

    virtual ~QNetworkStatePrivate()
    {
    }

    QString gateway() const
    {
        return gwSpace->value( "Default", QString() ).toString();
    }

    QList<QString> interfacesOnline;

Q_SIGNALS:
    void defaultGatewayChanged( const QString& iface );
    void connected();
    void disconnected();

private slots:

    void gatewayChanged()
    {
        emit defaultGatewayChanged( gateway() );
    }

    void update()
    {
        const bool isOnline = interfacesOnline.count();
        interfacesOnline.clear();
        const QStringList ifaceList = netSpace->subPaths();
        foreach( QString iface, ifaceList ) {
            QString config = netSpace->value( iface+QString("/Config") ).toString();
            if ( config.isEmpty() )
                continue;
            QtopiaNetworkInterface::Status state =
                (QtopiaNetworkInterface::Status) (netSpace->value(iface+QString("/State"), 0 ).toInt());
            if ( state == QtopiaNetworkInterface::Up
                || state == QtopiaNetworkInterface::Demand
                || state == QtopiaNetworkInterface::Pending ) {
                interfacesOnline << config;
            }
        }

        if ( isOnline && !interfacesOnline.count() )
            emit disconnected();
        else if ( !isOnline && interfacesOnline.count() )
            emit connected();
    }
private:
    QValueSpaceItem* netSpace;
    QValueSpaceItem* gwSpace;
};

/*!
  \class QNetworkState
  \brief
  The QNetworkState class provides information about the connectivity state of the Qtopia device.
*/

/*!
  Constructs a QNetworkState object with the specified \a parent.
*/
QNetworkState::QNetworkState( QObject* parent )
    : QObject( parent )
{
    d = new QNetworkStatePrivate( this );
    connect( d, SIGNAL(connected()), this, SIGNAL(connected()) );
    connect( d, SIGNAL(disconnected()), this, SIGNAL(disconnected()) );
    connect( d, SIGNAL(defaultGatewayChanged(const QString&)),
             this, SLOT(gatewayChanged(const QString&)) );
}

/*!
  Destructs the QNetworkState object.
*/
QNetworkState::~QNetworkState()
{
}

/*!
  Returns the handle to the network interface that is currently used as default gateway for
  network packages. If the network is offline the function returns an empty string.
*/
QString QNetworkState::gateway() const
{
    return d->gateway();
}

/*!
  This function returns the list of all interface  which are online at the time
  of the function call. The returned list contains the interfaces handles.
*/
QList<QString> QNetworkState::interfacesOnline() const
{
    return d->interfacesOnline;
}

/*!
  Returns the list of known network devices of type \a type.
  If \a type is \c{Any} it returns all known devices. A device is considered
  to be known if a configuration file exists for it. The returned Qtopia network interface handles
  are equivalent to the full qualified path to the configuration file.

 \sa QtopiaNetwork::availableNetworkConfigs()
*/
QList<QString> QNetworkState::availableNetworkDevices( QtopiaNetwork::Type type )
{
    return QtopiaNetwork::availableNetworkConfigs( type );
}

/*!
  Returns the type of \a devHandle.
*/
QtopiaNetwork::Type QNetworkState::deviceType( const QString& devHandle )
{
    return QtopiaNetwork::toType( devHandle );
}


/*!
  Returns the default WAP account. If no default account has been set this function returns
  an empty string.
*/
QString QNetworkState::defaultWapAccount() const
{
    QSettings cfg( "Trolltech", "Network" );
    cfg.beginGroup("WAP");
    QString r = cfg.value("DefaultAccount", QString()).toString();
    cfg.endGroup();
    return r;
}

/*!
  \fn void QNetworkState::connected()

  This signal is send when Qtopia changes from offline to online.
  If the connectivity state of a particular device is required QNetworkDevice::state
  should be used.

  \sa QtopiaNetwork::online()
*/

/*!
  \fn void QNetworkState::disconnected()

  This signal is send when Qtopia changes from online to offline.
  If the connectivity state of a particular device is required QNetworkDevice::state
  should be used.

  \sa QtopiaNetwork::online()
*/

/*!
  \fn void QNetworkState::defaultGatewayChanged( QString handle, const QNetworkInterface& local )

  This signal is emitted when the default gateway for network packages
  changes. Such a change usually implies that a new route and new DNS information
  have been set. \a handle is the new default interface and \a local contains the IP
  details for \a handle. An empty \a handle implies that no new default gateway is available. This
  may happen when the last online interface changes its state to offline.

  \sa gateway()
  */

/*!
  \internal
*/
void QNetworkState::gatewayChanged( const QString& newGateway )
{
    if ( newGateway.isEmpty() ) {
        QNetworkInterface iface;
        emit defaultGatewayChanged( QString(), iface );
    } else {
        QNetworkDevice dev( newGateway );
        if ( dev.state() == QtopiaNetworkInterface::Up ) {
            QNetworkInterface i = dev.address();
            emit defaultGatewayChanged( newGateway, i );
            return;
        }
        QNetworkInterface iface;
        emit defaultGatewayChanged( QString(), iface );
    }
}

#include "qnetworkstate.moc"
