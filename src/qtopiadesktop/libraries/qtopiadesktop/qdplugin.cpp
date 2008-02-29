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
#include <qdplugin.h>
#include <private/qdplugin_p.h>
#include <trace.h>
#include <qdlinkhelper.h>
#include <center.h>

#include <qcopchannel_qd.h>
#include <qcopenvelope_qd.h>
#include <qdebug.h>

// =====================================================================

#ifdef Q_OS_WIN32
/*!
  \internal
*/
CenterInterface::CenterInterface()
{
}
#endif

#ifdef Q_OS_UNIX
/*!
  \internal
*/
CenterInterface::~CenterInterface()
{
}
#endif

// =====================================================================

QDPluginData::QDPluginData()
    : center( 0 )
{
}

// =====================================================================

QDAppPluginData::QDAppPluginData()
    : appWidget( 0 ), settingsWidget( 0 ), mainWindow( 0 )
{
}

// =====================================================================

QDLinkPluginData::QDLinkPluginData()
    : helper( 0 )
{
}

// ====================================================================

/*!
  \class QDPluginFactory 
  \brief The QDPluginFactory class lets multiple plugins exist in a single file.

  Use of the QDPluginFactory should be automatic. You should not use this direcly but instead use
  the QD_REGISTER_PLUGIN() macro which will append your plugin to the list available from the current
  file.

  See \l{qdautoplugin.cpp} for the implementation used by the build system.

  \sa QDPlugin, {Qtopia Sync Agent Plugins}
*/

/*!
  Construct a QDPluginFactory with \a parent as the owning QObject.
*/
QDPluginFactory::QDPluginFactory( QObject *parent )
    : QObject( parent )
{
}

/*!
  Destructor.
*/
QDPluginFactory::~QDPluginFactory()
{
    TRACE(TRACE) << "QDPluginFactory::~QDPluginFactory";
}

/*!
  \fn QString QDPluginFactory::executableName() const
  Return the executable name of the plugin file. This is typically supplied by the build system
  in the QTOPIA_TARGET macro.
*/

/*!
  \fn QStringList QDPluginFactory::keys() const
  Return a list of identifiers that can be passed to the create function. 
*/

/*!
  \fn QDPlugin *QDPluginFactory::create( const QString &key )
  Create a plugin instance based on the \a key.
*/

// ====================================================================

/*!
  \class QDPlugin 
  \brief The QDPlugin class represents a Qtopia Sync Agent plugin.
  \mainclass

  All Qtopia Sync Agent plugins inherit from QDPlugin but the class itself is abstract.

  When creating a plugin you can use the QD_CONSTRUCT_PLUGIN macro to simplify the
  construction boilerplate that must be created.

  \sa <qdplugindefs.h>, {Qtopia Sync Agent Plugins}
*/

/*!
  Construct a QDPlugin with \a parent as the owning QObject.
*/
QDPlugin::QDPlugin( QObject *parent )
    : QObject( parent ), d( 0 )
{
}

/*!
  Destructor.
*/
QDPlugin::~QDPlugin()
{
    TRACE(TRACE) << "QDPlugin::~QDPlugin";
}

/*!
  Return the CenterInterface associated with this plugin.
*/
CenterInterface *QDPlugin::centerInterface()
{
    return d->center;
}

/*!
  \fn QDPlugin::id()
  Returns a unique value so that every plugin can be identified.
  It is recommended to use a reverse-DNS style name here.
  For example, com.trolltech.plugin.app.test
*/

/*!
  \fn QDPlugin::displayName()
  Returns the name the user will see when referring to this plugin.
*/


/*!
  This function can be used by plugins to do initialization. It is called after all plugins have been loaded
  so it is safe to request another plugin by it's id.
  
  Note that this function will not be called if your plugin is not enabled. You should ensure that your
  destructor will not fail as a result of skipping this method.
*/
void QDPlugin::init()
{
}

/*!
  \internal
*/
void QDPlugin::internal_init()
{
    if ( metaObject()->indexOfSlot("pluginMessage(const QString&,const QByteArray&)") != -1 ) {
        QCopChannel *chan = new QCopChannel( QString("QD/Plugin/%1").arg(id()), this );
        connect( chan, SIGNAL(received(QString,QByteArray)),
                 this, SLOT(pluginMessage(QString,QByteArray)) );
    }
}

/*!
  Lock the plugin using \a plugin as the key. A plugin can use locked() to see if it has been locked.

  This function is designed so that one plugin can inform another plugin that it is being used.
  
  For example, a device plugin is locked when a data transfer is occurring so that it does not
  disconnect.
*/
void QDPlugin::lock( QDPlugin *plugin )
{
    d->lockers << plugin;
    if ( qobject_cast<QDDevPlugin*>(this) )
        QCopEnvelope e( "QD/Connection", "setBusy()" );
}

/*!
  Remove the lock set by \a plugin.
*/
void QDPlugin::unlock( QDPlugin *plugin )
{
    d->lockers.removeAll(plugin);
    if ( d->lockers.count() == 0 )
        QCopEnvelope e( "QD/Connection", "clearBusy()" );
}

/*!
  Return true if anyone has locked this plugin.
*/
bool QDPlugin::locked() const
{
    return ( d->lockers.count() != 0 );
}

// ====================================================================

/*!
  \class QDAppPlugin
  \brief The QDAppPlugin class creates an application widget and a settings widget.
  \mainclass

  An App plugin must create an application widget (which appears in the main chooser).
  It can optionally create a settings widget (which appears in the settings dialog)
  and an info widget.

  \sa {Qtopia Sync Agent Plugins}
*/

/*!
  Construct a QDAppPlugin with \a parent as the owning QObject.
*/
QDAppPlugin::QDAppPlugin( QObject *parent )
    : QDPlugin( parent )
{
}

/*!
  Destructor.
*/
QDAppPlugin::~QDAppPlugin()
{
}

/*!
  \fn QDAppPlugin::id()
  \reimp
*/

/*!
  \fn QDAppPlugin::displayName()
  \reimp
*/

/*!
  \fn QDAppPlugin::icon()
  Return the icon that will be used to identify this plugin. It is helpful to choose a unique icon.
*/

/*!
  \fn QDAppPlugin::initApp()
  Create the app widget. The widget will be destroyed before the plugin is unloaded.
*/

/*!
  Create the settings widget. The widget will be destroyed before the plugin is unloaded.
*/
QWidget *QDAppPlugin::initSettings()
{
    return 0;
}

// ====================================================================

/*!
  \class QDLinkPlugin
  \brief The QDLinkPlugin class is responsible for maintaining a network link.
  \mainclass

  This plugin should be used to bring up a network interface and provide an IP address
  for Qtopia Sync Agent to connect to.

  \sa {Qtopia Sync Agent Plugins}
*/

/*!
  Construct a QDLinkPlugin with \a parent as the owning QObject.
*/
QDLinkPlugin::QDLinkPlugin( QObject *parent )
    : QDPlugin( parent )
{
}

/*!
  Destructor.
*/
QDLinkPlugin::~QDLinkPlugin()
{
    TRACE(TRACE) << "QDLinkPlugin::~QDLinkPlugin";
    QDLinkPluginData *dta = (QDLinkPluginData*)d;
    if ( dta && dta->helper ) {
        LOG() << "delete helper";
        delete dta->helper;
        dta->helper = 0;
    }
}

/*!
  Returns the socket for connections to use or 0 if the socket has not been initialized.
*/
QIODevice *QDLinkPlugin::socket()
{
    TRACE(QDLink) << "QDLinkPlugin::socket";
    QDLinkPluginData *dta = (QDLinkPluginData*)d;
    Q_ASSERT(dta);
    if ( dta->helper ) {
        QIODevice *device = dta->helper->socket();
        return device;
    }
    return 0;
}

/*!
  Call this function after you have created your \a device to initialize the socket.
  Note that the \a device will be moved to a separate thread so you should not interact
  with it except via signals. For the proxy device that you can interact with see
  QDLinkPlugin::socket().

  Ownership of \a device is taken by QDLinkPlugin and it will be destroyed when the plugin
  is destroyed but you can delete it yourself using the QObject::deleteLater() function.
*/
void QDLinkPlugin::setupSocket( QIODevice *device )
{
    TRACE(QDLink) << "QDLinkPlugin::setupSocket";
    QDLinkPluginData *dta = (QDLinkPluginData*)d;
    Q_ASSERT(dta && device && device->parent() == 0);
    Q_ASSERT(!dta->helper);
    dta->helper = new QDLinkHelper( device, this );
    // remove the helper if the device is destroyed
    connect( device, SIGNAL(destroyed()), this, SLOT(removeHelper()) );
}

/*!
  Returns the time in miliseconds between pings. Returning 0 will disable the pings,
  relying on the underlying connection to correctly indicate loss of connection.

  Note that this value can be overridden by the user when operating in debug mode.
*/
int QDLinkPlugin::ping_interval()
{
    return 5000;
}

/*!
  Returns true if \l HELPER_ACK commands should be sent, false otherwise.
*/
bool QDLinkPlugin::send_ack()
{
    return false;
}

/*!
  \internal
  Remove the QDLinkHelper object.
*/
void QDLinkPlugin::removeHelper()
{
    TRACE(QDLink) << "QDLinkPlugin::removeHelper";
    QDLinkPluginData *dta = (QDLinkPluginData*)d;
    Q_ASSERT(dta && dta->helper);
    delete dta->helper;
    dta->helper = 0;
}

/*!
  \fn QDLinkPlugin::id()
  \reimp
*/

/*!
  \fn QDLinkPlugin::displayName()
  \reimp
*/

/*!
  \fn bool QDLinkPlugin::tryConnect( QDConPlugin *connection )
  Attempt to bring up the link, using the port from \a connection (for TCP/IP-based links).
  Return false to indicate immediate failure and the setState() signal to indicate asynchronous failure.
*/

/*!
  \fn void QDLinkPlugin::stop()
  Tear down the link. The socket() function should return 0 once this function has been called.
*/

/*!
  \enum QDLinkPlugin::State
  This enum type specifies the state of the connection.
  \value Up The link is up.
  \value Down The link is down.
*/

/*!
  \fn void QDLinkPlugin::setState( QDLinkPlugin *plugin, int state )
  This signal should be emitted by the \a plugin to indicate changes in \a state
  using a constant from the QDLinkPlugin::State enum.
*/

// ====================================================================

/*!
  \class QDConPlugin
  \brief The QDConPlugin class is responsible for maintaining a connection.
  \mainclass

  This plugin should be used to establish a connection to a device.

  \sa {Qtopia Sync Agent Plugins}
*/

/*!
  Construct a QDConPlugin with \a parent as the owning QObject.
*/
QDConPlugin::QDConPlugin( QObject *parent )
    : QDPlugin( parent )
{
}

/*!
  Destructor.
*/
QDConPlugin::~QDConPlugin()
{
}

/*!
  \fn QDConPlugin::id()
  \reimp
*/

/*!
  \fn QDConPlugin::displayName()
  \reimp
*/

/*!
  \fn QString QDConPlugin::conProperty( const QString &key )
  Retrieve property \a key from the connection. Typically properties are set during the
  handshake phase.
*/

/*!
  \fn QDDevPlugin *QDConPlugin::device()
  Returns the device that has claimed this connection.
  \sa claim()
*/

/*!
  \fn bool QDConPlugin::claim( QDDevPlugin *dev )
  This is called by \a dev to attempt to claim the connection, which stops other plugins from getting control of it.
  The return value indicates success or failure.
  \sa device()
*/

/*!
  \fn void QDConPlugin::connected( QDDevPlugin *dev )
  Called by \a dev to report that handshaking is done. Note that \a dev is checked against the device that
  has claimed the connection.
  \sa claim()
*/

/*!
  \fn bool QDConPlugin::tryConnect( QDLinkPlugin *link )
  Attempt to connect using the socket from \a link.
  Return false to indicate immediate failure and the setState() signal to indicate asynchronous failure.
*/

/*!
  \fn void QDConPlugin::stop()
  Stop trying to connect to something.
*/

/*!
  \enum QDConPlugin::State
  This enum type specifies the state of the connection.
  \value Connected A connection has been established and any handshaking has been performed.
  \value Disconnected There is no connection.
  \value Connecting A connection attempt is in progress.
  \value Matching An attempt is being made to match this connection to a device.
*/

/*!
  \fn void QDConPlugin::setState( QDConPlugin *plugin, int state )
  This signal should be emitted by the \a plugin to indicate changes in \a state
  using a constant from the QDConPlugin::State enum.
*/

/*!
  \fn int QDConPlugin::port()
  Return the port to connect to (for TCP/IP-based communications).
*/

// ====================================================================

/*!
  \class QDDevPlugin
  \brief The QDDevPlugin class is responsible for communications with a device.
  \mainclass

  This plugin needs to work with a QDConPlugin in order to talk to a device.

  \sa {Qtopia Sync Agent Plugins}
*/

/*!
  Construct a QDDevPlugin with \a parent as the owning QObject.
*/
QDDevPlugin::QDDevPlugin( QObject *parent )
    : QDPlugin( parent )
{
}

/*!
  Destructor.
*/
QDDevPlugin::~QDDevPlugin()
{
}

/*!
  \fn QDDevPlugin::id()
  \reimp
*/

/*!
  \fn QDDevPlugin::displayName()
  \reimp
*/

/*!
  \fn QString QDDevPlugin::model()
  Return the model name of the device. For example, "Trolltech Greenphone".
*/

/*!
  \fn QString QDDevPlugin::system()
  Return the system that runs on the device. This is typically "Qtopia" but a fork
  could use their own name (eg. OPIE).
*/

/*!
  \fn quint32 QDDevPlugin::version()
  Return the version of the device in a hexidecimal format. For example, 0x040300 would
  be used for a version 4.3.0 device.
*/

/*!
  \fn QString QDDevPlugin::versionString()
  Return the version of the device as a string that can be displayed to the user.
*/

/*!
  \fn QPixmap QDDevPlugin::icon()
  Return an image that can be used as an icon for the device. This is used so the user can see
  what they have connected to Qtopia Sync Agent.
*/

/*!
  \fn void QDDevPlugin::probe( QDConPlugin *con )
  Probe the connection \a con to see if a device of this type can be reached via that connection.
*/

/*!
  \fn void QDDevPlugin::disassociate( QDConPlugin *con )
  Remove the association between this device and connection \a con. This will trigger the connection
  to report that it is no longer connected.
*/

// ====================================================================

/*!
  \class QDSyncPlugin 
  \brief The QDSyncPlugin class represents a Qtopia Sync Agent sync plugin.
  \mainclass

  All Qtopia Sync Agent sync plugins inherit from QDSyncPlugin but the class itself is abstract.

  \sa QDPlugin
*/

/*!
  Construct a QDSyncPlugin with \a parent as the owning QObject.
*/
QDSyncPlugin::QDSyncPlugin( QObject *parent )
    : QDPlugin( parent )
{
}

/*!
  Destructor.
*/
QDSyncPlugin::~QDSyncPlugin()
{
}

/*!
  \fn QDSyncPlugin::id()
  \reimp
*/

/*!
  \fn QDSyncPlugin::displayName()
  \reimp
*/

/*!
  Returns an example xml schema for the records handled by the plugin. This schema
  should represent the fields handled by the plugin and is used to improve
  conflict detection by the merge algorithm.

  This method should be implemented by plugins that do not support the entire schema.
*/
QByteArray QDSyncPlugin::referenceSchema()
{
    return QByteArray();
}

/*!
  This method is called when the plugin is about to be used. The plugin
  should allocate any required resources and should indicate readyness
  using the readyForSync() signal. This can be done asynchronously but
  it must be done or the sync will stall.
*/
void QDSyncPlugin::prepareForSync()
{
    emit readyForSync( true );
}

/*!
  This method is called when the plugin has finished it's sync.
  The plugin should release any resources no longer needed and
  indicate completion using the finishedSync() signal. This can be
  done asynchronously but it must be done or the sync will stall.

  Note that any objects assigned to the object returned by
  CenterInterface::syncObject() will not be deleted until after
  the sync has completed.
*/
void QDSyncPlugin::finishSync()
{
    emit finishedSync();
}

/*!
  \fn QString QDSyncPlugin::dataset()
  Returns the dataset that this sync plugin supports.
  \sa Datasets
*/

/*!
  \fn void QDSyncPlugin::readyForSync( bool ready )
  This signal should be emitted after prepareForSync() is called to indicate
  that the plugin is \a ready. Use false to indicate that the plugin cannot sync
  at this time (for example, if the data source is not available).
*/

/*!
  \fn void QDSyncPlugin::finishedSync()
  This signal should be emitted after finishSync() is called to indicate
  that the plugin has cleaned up its resources.
*/

// ====================================================================

/*!
  \class QDClientSyncPlugin 
  \brief The QDClientSyncPlugin class represents a sync client.
  \mainclass

  The sync implementation uses both client and server plugins. A typical client plugin
  is expected to pass messages between a real device and the sync implementation, doing
  no actual work itself.
*/

/*!
  Construct a QDClientSyncPlugin with \a parent as the owning QObject.
*/
QDClientSyncPlugin::QDClientSyncPlugin( QObject *parent )
    : QDSyncPlugin( parent )
{
}

/*!
  Destructor.
*/
QDClientSyncPlugin::~QDClientSyncPlugin()
{
}

/*!
  \fn QDClientSyncPlugin::id()
  \reimp
*/

/*!
  \fn QDClientSyncPlugin::displayName()
  \reimp
*/

/*!
  \fn QString QDClientSyncPlugin::dataset()
  \reimp
*/

/*!
  \fn void QDClientSyncPlugin::serverSyncRequest(const QString &source)
  Notify the client that the server has requested a sync with datasource \a source.
*/

/*!
  \fn void QDClientSyncPlugin::serverIdentity(const QString &server)
  Notify the client that the server's identity is \a server.
*/

/*!
  \fn void QDClientSyncPlugin::serverVersion(int major, int minor, int patch)
  Notify the client that the server's version is equivalent to the values \a major, \a minor and \a patch.
*/

/*!
  \fn void QDClientSyncPlugin::serverSyncAnchors(const QDateTime &serverLastSync, const QDateTime &serverNextSync)
  Notify the client that the server's sync anchors are \a serverLastSync and \a serverNextSync.
*/

/*!
  \fn void QDClientSyncPlugin::createServerRecord(const QByteArray &record)
  Notify the client that the server has created \a record.
*/

/*!
  \fn void QDClientSyncPlugin::replaceServerRecord(const QByteArray &record)
  Notify the client that the server has replaced \a record.
*/

/*!
  \fn void QDClientSyncPlugin::removeServerRecord(const QString &serverId)
  Notify the client that the server has removed the record with id \a serverId.
*/

/*!
  \fn void QDClientSyncPlugin::requestTwoWaySync()
  Notify the client that the server wants to perform a two-way sync.
*/

/*!
  \fn void QDClientSyncPlugin::requestSlowSync()
  Notify the client that the server wants to perform a slow sync.
*/

/*!
  \fn void QDClientSyncPlugin::serverError()
  Notify the client that the server has encountered an error.
*/

/*!
  \fn void QDClientSyncPlugin::serverEnd()
  Notify the client that the server has finished.
*/

/*!
  \fn void QDClientSyncPlugin::clientSyncRequest(const QString &source)
  Notify the sync manager that the client wants to request a sync with dataset \a source.
*/

/*!
  \fn void QDClientSyncPlugin::clientIdentity(const QString &id)
  Notify the sync manager that the client's identity is \a id.
*/

/*!
  \fn void QDClientSyncPlugin::clientVersion(int major, int minor, int patch)
  Notify the sync manager that the client's version is equivalent to the values \a major, \a minor and \a patch.
*/

/*!
  \fn void QDClientSyncPlugin::clientSyncAnchors(const QDateTime &clientLastSync, const QDateTime &clientNextSync)
  Notify the sync manager that the client's sync anchors are \a clientLastSync and \a clientNextSync.
*/

/*!
  \fn void QDClientSyncPlugin::createClientRecord(const QByteArray &record)
  Notify the sync manager that the client has created \a record.
*/

/*!
  \fn void QDClientSyncPlugin::replaceClientRecord(const QByteArray &record)
  Notify the sync manager that the client has replaced \a record.
*/

/*!
  \fn void QDClientSyncPlugin::removeClientRecord(const QString &clientId)
  Notify the sync manager that the client has removed the record with id \a clientId.
*/

/*!
  \fn void QDClientSyncPlugin::mapId(const QString &serverId, const QString &clientId)
  Notify the sync manager that the client has mapped the server identifier \a serverId to the client identifier \a clientId.
*/

/*!
  \fn void QDClientSyncPlugin::clientError()
  Notify the sync manager that the client has encountered an error.
*/

/*!
  \fn void QDClientSyncPlugin::clientEnd()
  Notify the sync manager that the client has finished.
*/

// ====================================================================

/*!
  \class QDServerSyncPlugin 
  \brief The QDServerSyncPlugin class represents a sync server.
  \mainclass

  The sync implementation uses both client and server plugins. A typical server plugin
  is expected to fetch changes from and submit changes back to the data store of another
  application (either directly or via some third party API).
*/

/*!
  Construct a QDClientSyncPlugin with \a parent as the owning QObject.
*/
QDServerSyncPlugin::QDServerSyncPlugin( QObject *parent )
    : QDSyncPlugin( parent )
{
}

/*!
  Destructor.
*/
QDServerSyncPlugin::~QDServerSyncPlugin()
{
}

/*!
  \fn QDServerSyncPlugin::id()
  \reimp
*/

/*!
  \fn QDServerSyncPlugin::displayName()
  \reimp
*/

/*!
  \fn QString QDServerSyncPlugin::dataset()
  \reimp
*/

/*!
  \fn void QDServerSyncPlugin::performSync(const QDateTime &timestamp)
  Perform a sync on items modified after \a timestamp. The work can be done asynchronously
  and progress should be indicated via signals.
  \sa createServerRecord(), replaceServerRecord(), removeServerRecord(), serverChangesCompleted()
*/

/*!
  \fn void QDServerSyncPlugin::createClientRecord(const QByteArray &record)
  Create the item specified in \a record.
  \sa mappedId()
*/

/*!
  \fn void QDServerSyncPlugin::replaceClientRecord(const QByteArray &record)
  Update the item specified in \a record.
*/

/*!
  \fn void QDServerSyncPlugin::removeClientRecord(const QString &identifier)
  Remove the item indicated by \a identifier.
*/

/*!
  \fn void QDServerSyncPlugin::mappedId(const QString &serverId, const QString &clientId)
  Emit this signal when a client id (\a clientId) can be mapped to a server id (\a serverId).
  \sa createClientRecord()
*/

/*!
  \fn void QDServerSyncPlugin::createServerRecord(const QByteArray &record)
  Emit this signal when a new item has been created. The item is passed via \a record.
  \sa performSync()
*/

/*!
  \fn void QDServerSyncPlugin::replaceServerRecord(const QByteArray &record)
  Emit this signal when an item has been modified. The item is passed via \a record.
  \sa performSync()
*/

/*!
  \fn void QDServerSyncPlugin::removeServerRecord(const QString &identifier)
  Emit this signal when an item has been removed. The identifier of the item is passed via \a identifier.
  \sa performSync()
*/

/*!
  \fn void QDServerSyncPlugin::serverChangesCompleted()
  Emit this signal when the item fetching is complete.
  \sa performSync()
*/

// ====================================================================

QD_EXPORT QList<qdPluginCreateFunc_t> *qdInternalPlugins();
QList<qdPluginCreateFunc_t> *qdInternalPlugins()
{
    static QList<qdPluginCreateFunc_t> *internalPlugins = new QList<qdPluginCreateFunc_t>;
    return internalPlugins;
}

QD_EXPORT void qd_registerPlugin(qdPluginCreateFunc_t createFunc);
void qd_registerPlugin(qdPluginCreateFunc_t createFunc)
{
    //qDebug() << "qd_registerPlugin called";
    qdInternalPlugins()->append(createFunc);
}

