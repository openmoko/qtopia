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

#include "qtopianetwork.h"
#include "qtopianetworkinterface.h"

#include <QCoreApplication>
#include <QSettings>

#include <qvaluespace.h>
#include <qpluginmanager.h>
#include <qtopiaipcadaptor.h>
#include <qtopialog.h>
#include <qtopianamespace.h>

/*!

  \class QtopiaNetwork

  This class provides static convinience functions for various network parameter.

  In general it allows the managing of existing network configurations.
  The managment functions include the starting and stopping of interfaces and
  other functions which may be useful for applications which monitor the
  connectivity state of the Qtopia device.
*/

/*!
  \enum QtopiaNetwork::TypeFlag

  This enum is used to describe the type of a network connection.

  \value LAN Ethernet based network.
  \value WirelessLAN Wireless network.
  \value Dialup The connection is established via a dial-up connection.
  \value GPRS The connection is established via GPRS/UMTS/EDGE.
  \value Bluetooth The network is based on Bluetooth.
  \value Hidden This interface is hidden from the user. It usually represents a
                network interface that offers a network service implemented by a network plugin.
                Such an interface is automatically started/stopped by the network server. An example
                of such an interface would be the Qtopia Bluetooth DUN service.
  \value Any A place holder for any arbitrary network type.

  These sub types are used in conjunction with QtopiaNetwork::GPRS and
  QtopiaNetwork::Dialup only:

  \value PhoneModem The network connection is established via the internal phone
             modem (this flag is defined for Qtopia Phone Edition only).
  \value NamedModem A serial network connection using a specific device
             such as /dev/ttyS0 is established. The device name is given in the
             configuration file.

  These sub types specify the type of the external device which is used to
  establish the connection.

   \value PCMCIA The network device is an attached PCMCIA card

  These sub types are used in conjunction with QtopiaNetwork::Bluetooth only.

  \value BluetoothDUN The network connection is established via a local bluetooth
            device. The remote Bluetooth device acts as Internet gateway as specified
            by the Dial-up Networking Profile (DNP).
*/

/*!
  Starts that the network interface identified by \a handle. If the interface is running allready
  this function does nothing.

  \a options is used internally by some network plugins.
  It allows to specify additional parameters which are specific to particular network plugins.
  Otherwise this parameter can safely be ignored.
*/
void QtopiaNetwork::startInterface( const QString& handle, const QVariant& options )
{
    QtopiaIpcAdaptor o("QPE/Network");
    o.send( MESSAGE(startInterface(const QString&,const QString&,const QVariant&)),
            qApp ? qApp->applicationName() : QString(), handle, options );
}

/*!
  Stops the network interface identified by \a handle. If \a deleteIface is true
  the server will delete all configuration information associated to this interface.
*/
void QtopiaNetwork::stopInterface( const QString& handle, bool deleteIface)
{
    QtopiaIpcAdaptor o("QPE/Network");
    o.send( MESSAGE(stopInterface(const QString&,const QString&,bool)),
            qApp ? qApp->applicationName() : QString(), handle, deleteIface );
}

/*!
  \internal

  This function allows the stopping of network connection w/o considering other applications
  which might have an interest in the same connection. This is achieved by circumventing
  the network session manager (and thus ignoring the extended life time flag). In general
  this should not be used by applications other than the global network manager. Qtopia's
  netsetup application uses this function to allow the user to stop a network connection
  which could have been started by rogue applications running in the background.
  */
void QtopiaNetwork::privilegedInterfaceStop( const QString& handle )
{
    QtopiaIpcAdaptor o("QPE/Network");
    o.send( MESSAGE(privilegedInterfaceStop(const QString&)), handle );
}

/*!
  Stops all active network interfaces.
*/
void QtopiaNetwork::shutdown()
{
    QtopiaIpcAdaptor o("QPE/Network");
    o.send( MESSAGE(shutdownNetwork()) );
}

/*!
  Sets the network interface identified by \a handle as default gateway. If the
  device is connected to more than one network at a time this can be used to
  choose a preferred network for data transfers.
*/
void QtopiaNetwork::setDefaultGateway( const QString& handle )
{
    QtopiaIpcAdaptor o("QPE/Network");
    o.send( MESSAGE(setDefaultGateway(const QString&,bool)), handle, false );
}

/*!
  Qtopia will set the default gateway to an interface that
  is not equal to \a handle. This is useful if the interface identified by \a handle goes offline and the caller doesn't care
  what interface should become the new default gateway.
  */
void QtopiaNetwork::unsetDefaultGateway( const QString& handle )
{
    QtopiaIpcAdaptor o("QPE/Network");
    o.send( MESSAGE(setDefaultGateway(const QString&,bool)), handle, true );
}

/*!
  \internal

  If \a isLocked is true the network server initiates shutdown() and prevents
  new network connections from being established.
  */
void QtopiaNetwork::lockdown( bool isLocked )
{
    QtopiaIpcAdaptor o("QPE/Network");
    o.send( MESSAGE(setLockMode(bool)), isLocked );
}

/*!
  \internal

  Sets the life time of the interface \a handle to \a isExtended. If \a isExtended is \c TRUE
  \a handle will remain online indefinitly. Usually the session manager keeps track of interfaces
  started by applications and closes the connection when the application quits.

  This function can only be called by the Qtopia server and the netsetup application (enforced by SXE).
  */
void QtopiaNetwork::extendInterfaceLifetime( const QString& handle, bool isExtended )
{
    QtopiaIpcAdaptor o("QPE/Network");
    o.send( MESSAGE(setExtendedInterfaceLifetime(const QString&,bool)), handle, isExtended );
}

/*!
  Returns the default directory for network interface configurations.
*/
QString QtopiaNetwork::settingsDir()
{
    return Qtopia::applicationFileName("Network", "config");
}

/*!
  Returns the type of network interface defined by \a handle.
*/
QtopiaNetwork::Type QtopiaNetwork::toType(const QString& handle)
{
    QSettings cfg( handle, QSettings::IniFormat );
    cfg.sync();
    const QString tp = cfg.value("Info/Type").toString();

    QtopiaNetwork::Type t;
    if ( tp == "dialup" )
    {
        if ( cfg.value("Serial/Type").toString() == "external" )
        {
            if ( cfg.value("Serial/SerialDevice").toString().isEmpty() )
                t |= QtopiaNetwork::PCMCIA;
            else
                t |= QtopiaNetwork::NamedModem;
        }
#ifdef QTOPIA_CELL
        else
        {
            t |= QtopiaNetwork::PhoneModem;
        }
#endif
        if ( cfg.value("Serial/GPRS").toString() == "y" )
            t |= QtopiaNetwork::GPRS;
        else
            t |= QtopiaNetwork::Dialup;
    }
    else if ( tp == "lan" )
    {
        if ( cfg.value("Properties/DeviceType").toString() == "wlan" )
            t |= QtopiaNetwork::WirelessLAN;
        else if ( cfg.value("Properties/DeviceType").toString() == "eth" )
            t |= QtopiaNetwork::LAN;
    }
    else if ( tp == "pcmcialan" )
    {
        t |= QtopiaNetwork::PCMCIA;
        if ( cfg.value("Properties/DeviceType").toString() == "wlan" )
            t |= QtopiaNetwork::WirelessLAN;
        else if ( cfg.value("Properties/DeviceType").toString() == "eth" )
            t |= QtopiaNetwork::LAN;
    }
    else if ( tp == "bluetooth" )
    {
        t |= QtopiaNetwork::Bluetooth;
        QByteArray a = cfg.value("Bluetooth/Profile").toByteArray();
        if ( a == "DUN" )
            t |= QtopiaNetwork::BluetoothDUN;
    }

    if ( cfg.value("Info/Visibility").toByteArray() == "hidden" ) {
        t |= QtopiaNetwork::Hidden;
    }

    return t;
}

/*!
  Returns a list of all known Qtopia network interface handles which match type \a t. A handle uniquely identifies all
  Qtopia network interfaces. A handle is the absolute path to the configuration file
  that contains the settings for the particular QtopiaNetworkInterfaces.
  \a path specifies the directory where the lookup takes place.

  Note that the lookup is independent of the state of an interface. It always returns all
  Qtopia network interfaces which are online and offline.

  If \a t is \c{Any} it returns all known NetworkInterfaces. If \a path is
  empty the lookup will take place in settingsDir().
*/
QStringList QtopiaNetwork::availableNetworkConfigs( QtopiaNetwork::Type t,
        const QString& path)
{
    QStringList resultList;

    QString fileName = path;
    if ( path.isEmpty() )
        fileName = settingsDir();

    //qLog(Network) << "QN: Searching for configs in " << fileName;
    QDir configDir(fileName);
    configDir.mkdir(fileName);

    QStringList files = configDir.entryList(QStringList("*.conf"));
    foreach(QString entry, files)
    {
        entry = configDir.filePath(entry);
        QtopiaNetwork::Type configType = toType( entry );
        if ( ( configType & t) == t )
            resultList.append( entry );
    }

    if ( resultList.isEmpty() )
        qLog(Network) << "QN: no configuration available";
    else
        qLog(Network) << "QN: Found " << resultList;
    return resultList;
}

static QHash<QString,QPointer<QtopiaNetworkInterface> > *loadedIfaces = 0;
static QHash<int,QtopiaNetworkFactoryIface*> *knownPlugins = 0;
static QPluginManager *pmanager = 0;

static void cleanup()
{
    if (loadedIfaces)
        delete loadedIfaces;
    if (knownPlugins)
        delete knownPlugins;
    if (pmanager)
        delete pmanager;
    loadedIfaces = 0;
    knownPlugins = 0;
    pmanager = 0;
}


/*!
     Loads the appropriate network plug-in for the interface with \a handle.
*/
QPointer<QtopiaNetworkInterface> QtopiaNetwork::loadPlugin( const QString& handle)
{
    if ( handle.isEmpty() || !QFile::exists(handle) ) {
        qLog(Network) << "QN::loadPlugin(): Invalid settings file passed" << handle;
        return 0;
    }

#ifndef QT_NO_COMPONENT
    QtopiaNetworkFactoryIface *plugin = 0;
    QPointer<QtopiaNetworkInterface> impl = 0;

    if ( !loadedIfaces ) {
        loadedIfaces = new QHash<QString, QPointer<QtopiaNetworkInterface> >;
        qAddPostRoutine(cleanup);
    }
    if ( !knownPlugins )
        knownPlugins = new QHash<int,QtopiaNetworkFactoryIface*>;
    if ( !pmanager )
        pmanager = new QPluginManager( "network" ); //no tr

    //check that we do not already have an instance for this config
    QHash<QString, QPointer<QtopiaNetworkInterface> >::const_iterator iter =
        loadedIfaces->find( handle );
    if ( iter != loadedIfaces->end() )
    {
        if (iter.value() != 0) {
            impl = iter.value();
            //qLog(Network) << "QN::loadPlugin() : interface already in cache, returning instance";
        } else {
            //qLog(Network) << "QN::loadPlugin() : interface deleted, removing bogus reference";
            loadedIfaces->remove( handle ); //instance has been deleted already
        }
    }

    if ( !impl )
    {
        Type t = toType(handle);
        //go through list of all known plugins
        int found = 0;
        foreach ( int key, knownPlugins->keys() )
        {
            if ( (key & t)==t ) {
               found = key;
               break;
            }
        }

        if ( found ) {
            //qLog(Network) << "QN::loadPlugin() : plugin in cache, creating new interface instance";
            plugin = knownPlugins->value(found);
            impl = plugin->network( handle );
            loadedIfaces->insert( handle, impl );

        } else {
            // the interface hasn't been instanciated nor do we have a
            // suitable plug-in yet. Go through all plug-ins
            //qLog(Network) << "QN::loadPlugin() : Extensive search mode";
            QStringList pluginList = pmanager->list();
            foreach(QString item, pluginList)
            {
                QObject *instance = pmanager->instance(item);
                plugin = qobject_cast<QtopiaNetworkFactoryIface*>(instance);
                if ( plugin && ((plugin->type() & t) == t) )
                {
                    qLog(Network) << "QN::loadPLugin() : plugin found,"
                                  << "loaded and new interface instanciated";
                    knownPlugins->insert( plugin->type(), plugin );
                    impl = plugin->network( handle );
                    loadedIfaces->insert( handle, impl );
                    break;
                }
            }
        }
    }
    if (!impl)
        qLog(Network) << "QN::loadPlugin(): no suitable plugin found";
    return impl;
#else
    return 0;
#endif
}

/*!
    Returns true if any known network interface is online/connected.
    This can be used to check the general connectivity status of the Qtopia environment.
    A device is considered to be online if it is either Up, Demand or Pending.

    Dynamic state updates on a per device base can be obtained by using QNetworkDevice.
*/
bool QtopiaNetwork::online()
{
    QValueSpaceItem netSpace( "/Network/Interfaces" );
    const QList<QString> ifaceList = netSpace.subPaths();
    foreach( QString iface, ifaceList ) {
        QtopiaNetworkInterface::Status state =
            (QtopiaNetworkInterface::Status) (netSpace.value( iface+QByteArray("/State"), 0 )).toInt();
        switch (state) {
            case QtopiaNetworkInterface::Up:
            case QtopiaNetworkInterface::Demand:
            case QtopiaNetworkInterface::Pending:
                return true;
                break;
            default:
                break;
        }
    }
    return false;
}
