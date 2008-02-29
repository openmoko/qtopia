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

#include <qtopianetworkinterface.h>
#include <qtopialog.h>

/*!
    \class QtopiaNetworkInterface
    \brief The QtopiaNetworkInterface class describes the minimum interface that
    a network plug-in must provide.

    There is no default implementation available for this interface.

  \ingroup io
*/

/*!
  \enum QtopiaNetworkInterface::Status

  The status provides information about the state of the network interface.

  \value Unknown this is the initial state of a network interface
      that has not been initialized yet (\c{initialize()}). The interface
      must never return to this state after initialization.
  \value Down the interface is ready to be started
  \value Up the interface is up and running
  \value Pending temporary state of the interface
  \value Demand Initiate the link only on demand when data traffic is present
  \value Unavailable the interface cannot be started because
      the network device does not exist and/or cannot be recognised

*/

/*!
  \enum QtopiaNetworkInterface::Error

  This enum identifies the error that occurred.

  \value NoError No error occurred.
  \value NotConnected The device could not connect to the network or in case of
      of a PPP connection to the peer server.
  \value NotInitialized The network plug-in has not been initialized yet. The interface can
      be initialized by calling QtopiaNetworkInterface::initialize().
  \value NotAvailable The device is not available (e.g. the PCMCIA card is not plugged in).
  \value UnknownError An error other than those specified above occurred.
*/

/*!
  \fn bool QtopiaNetworkInterface::start( const QVariant options = QVariant() )

  Starts the network service, returning true on success.
  \a options is used to pass optional parameter to the interface. The actual value of \a options is plug-in specific and
  plug-in developer may use it for any arbitrary value that might be of importance to the startup code.

  Note: This function may only be called by the qpe server. Qtopia
  applications should start an interface via
  QtopiaNetwork::startInterface().
 */

/*!
  \fn bool QtopiaNetworkInterface::stop()

  Stops the network service,
  returning true on success.

  Note: This function may only be called by the qpe server. Qtopia
  applications should stop a specific interface via
  QtopiaNetwork::stopInterface() or QtopiaNetwork::shutdown()
 */

/*!
  \fn Status QtopiaNetworkInterface::status()

  Updates and returns the current status of the interface. At any given time
  a call to this function should always perform a status check before
  returning the results.

  \sa QtopiaNetworkInterface::Status
 */

/*!
  \fn void QtopiaNetworkInterface::initialize()

  Initializes the network interface configuration.
  The implementation must change the interface state
  to a value that is not equal to QtopiaNetworkInterface::Unknown.
 */

/*!
  \fn void QtopiaNetworkInterface::cleanup()

  Cleans up all system and config files (e.g. the dialup plug-in deletes
  the peer and connect/disconnect chat file) created for this interface. However this function must not remove the
  network configuration file associated with this plug-in instance as this will be
  done by the network server.
 */

/*!
  \fn void QtopiaNetworkInterface::setDefaultGateway()

  This interface becomes the default gateway for the device.
 */

/*!
  \fn QString QtopiaNetworkInterface::device() const

  Returns the name of the network device that is used for connections.
  (e.g. eth0, wlan1, ppp0). This name is only valid if the interface
  status is not QtopiaNetworkInterface::Unknown or QtopiaNetworkInterface::Unavailable
 */

/*!
  \fn QtopiaNetwork::Type QtopiaNetworkInterface::type() const

  Returns the type of network interface that this particular interface can handle.
 */

/*!
  \fn QtopiaNetworkConfiguration *QtopiaNetworkInterface::configuration()

  Allows access to the configuration for this interface.
 */

/*!
  \fn void QtopiaNetworkInterface::setProperties( const QtopiaNetworkProperties& properties)

  The content of \a properties is written to the network configuration for this interface.
 */

/*!
  \internal

  \fn QtopiaNetworkMonitor QtopiaNetworkInterface::monitor()

  This function call is a placeholder for future releases of Qtopia. A Qtopia network monitor
  will perform operations such as traffic and cost monitoring.
 */

/*----------------------------------------------------------*/
// QtopiaNetworkMonitor

/*!
  \internal
  \class QtopiaNetworkMonitor
  \brief The QtopiaNetworkMonitor class allows the monitoring of
  data traffic and can calculate the associated costs

  \warning This class merely serves as a placeholder for future
  releases of Qtopia.
*/

/*----------------------------------------------------------*/
// QtopiaNetworkConfiguration

/*!
  \class QtopiaNetworkConfiguration
  \brief The QtopiaNetworkConfiguration class defines the interface to
  network interface configurations.

  Network interface properties can either changed via writeProperties()
  or by calling configure() which returns a user dialog.

  There is no default implementation available for this interface.

  \ingroup io
 */

/*!
  \fn QString QtopiaNetworkConfiguration::configFile() const

  Returns the absolute path and filename of the network configuration file that defines
  the current interface. The return string can also be used as interface handle.
 */

/*!
  \fn QVariant QtopiaNetworkConfiguration::property(const QString& key) const

  This is a quick way of accessing the configuration value for
  \a key.
 */

/*!
  \fn QStringList QtopiaNetworkConfiguration::types() const

  Returns a list all user interfaces. Each list entry is a localised string and can by used as a
  short description for the interface. The first entry in the list is the default configuration interface.
  The returned list must contain at least one entry (being the default interface).
 */

/*!
  \fn QDialog* QtopiaNetworkConfiguration::configure(QWidget *parent, const QString& type)

  Returns the default configuration dialog that is shown to the user when
  he configures this interface. \a parent is the parent widget for the dialog.
  The valid values to \a type can be obtained by using types().
  If \a type is unknown this function returns 0. The default configuration interface is returned
  if no type is passed.

  \bold{Note:} A plug-in can have additional configuration interfaces which are relevant to
  the plug-in but not essential. For example, the LAN plug-in has an additional user interfaces
  for WLAN scanning.

  \sa types()
 */

/*!
  Deconstructor.
  */
QtopiaNetworkConfiguration::~QtopiaNetworkConfiguration()
{
}

/*!
  \fn QtopiaNetworkProperties QtopiaNetworkConfiguration::getProperties() const

  Returns all properties of this interface. If only a single property is needed
  QtopiaNetworkConfiguration::property() should be used.

  \sa property()
 */

/*!
  \fn void QtopiaNetworkConfiguration::writeProperties( const QtopiaNetworkProperties& properties)

  Write \a properties out to the network configuration file.
 */

/*----------------------------------------------------------*/
// QtopiaNetworkProperties

/*!
  \class QtopiaNetworkProperties
  \brief The QtopiaNetworkProperties class reflects the content of a network interface configuration file.

  It is used as a generic way to access settingsfiles by mapping keys to
  values.

  \ingroup io
*/

/*!
  \fn QtopiaNetworkProperties::QtopiaNetworkProperties()

  Constructs an empty property map.
*/

/*!
  \fn QtopiaNetworkProperties::QtopiaNetworkProperties( const QtopiaNetworkProperties & other)

  Constructs a copy of \a other.
*/

/*!
  \fn QtopiaNetworkProperties::~QtopiaNetworkProperties()

  Destroys the object.
*/


QtopiaNetworkProperties::QtopiaNetworkProperties()
{
}

QtopiaNetworkProperties::QtopiaNetworkProperties( const QtopiaNetworkProperties& list )
    : QMap<QString, QVariant> ( list )
{
}

QtopiaNetworkProperties::~QtopiaNetworkProperties()
{
}


/*!
  This function dumps the content to the debug stream.
*/
void QtopiaNetworkProperties::debugDump() const
{
    QString key;
    QString value;


    QtopiaNetworkProperties::const_iterator i = constBegin();
    qLog(Network) << "Dumping network settings";
    while ( i != constEnd() )
    {
        qLog(Network) << i.key() << ": " << i.value().toString();
        i++;
    }

}

/*----------------------------------------------------------*/
//QtopiaNetworkPlugin

/*!
  Constructor.
*/
QtopiaNetworkPlugin::QtopiaNetworkPlugin(QObject* parent)
    :QObject(parent)
{
}

/*!
  Deconstructor.
*/
QtopiaNetworkPlugin::~QtopiaNetworkPlugin()
{
}

/*!
  Deconstructor.
*/
QtopiaNetworkInterface::~QtopiaNetworkInterface()
{
}

QtopiaNetworkMonitor* QtopiaNetworkInterface::monitor()
{
    return 0;
}

/*!
  Deconstructor.
*/
QtopiaNetworkFactoryIface::~QtopiaNetworkFactoryIface()
{
}

