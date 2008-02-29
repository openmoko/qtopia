/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qdebug.h>
#include <qcoreapplication.h>
#include <qstringlist.h>

#include "qdbusconnection.h"
#include "qdbusconnectioninterface.h"
#include "qdbuserror.h"
#include "qdbusmessage.h"
#include "qdbusmessage_p.h"
#include "qdbusconnection_p.h"
#include "qdbusinterface_p.h"
#include "qdbusutil_p.h"

class QDBusConnectionManager
{
public:
    QDBusConnectionManager() {}
    ~QDBusConnectionManager();
    void bindToApplication();

    QDBusConnectionPrivate *connection(const QString &name) const;
    void removeConnection(const QString &name);
    void setConnection(const QString &name, QDBusConnectionPrivate *c);

    QDBusConnectionPrivate *sender() const;
    void setSender(const QDBusConnectionPrivate *s);

private:
    mutable QMutex mutex;
    QHash<QString, QDBusConnectionPrivate *> connectionHash;

    mutable QMutex senderMutex;
    QString senderName; // internal; will probably change
};

Q_GLOBAL_STATIC(QDBusConnectionManager, _q_manager)

QDBusConnectionPrivate *QDBusConnectionManager::sender() const
{
    QMutexLocker locker(&senderMutex);
    return connection(senderName);
}

void QDBusConnectionManager::setSender(const QDBusConnectionPrivate *s)
{
    QMutexLocker locker(&senderMutex);
    senderName = (s ? s->name : QString());
}

QDBusConnectionPrivate *QDBusConnectionManager::connection(const QString &name) const
{
    QMutexLocker locker(&mutex);
    return connectionHash.value(name, 0);
}

void QDBusConnectionManager::removeConnection(const QString &name)
{
    QMutexLocker locker(&mutex);

    QDBusConnectionPrivate *d = 0;
    d = connectionHash.take(name);
    if (d && !d->ref.deref()) {
        delete d;
    } else if (d) {
        d->closeConnection();
        qWarning("QDBusConnection: closed connection %s"
                 "is still referred to by other QDBusConnection objects",
                 name.toLocal8Bit().constData());
    }
}

QDBusConnectionManager::~QDBusConnectionManager()
{
    for (QHash<QString, QDBusConnectionPrivate *>::const_iterator it = connectionHash.constBegin();
         it != connectionHash.constEnd(); ++it) {
        QDBusConnectionPrivate *d = it.value();
        if (!d->ref.deref())
            delete d;
        else
            d->closeConnection();
    }
    connectionHash.clear();
}

void QDBusConnectionManager::bindToApplication()
{
    QMutexLocker locker(&mutex);
    for (QHash<QString, QDBusConnectionPrivate *>::const_iterator it = connectionHash.constBegin();
         it != connectionHash.constEnd(); ++it) {
             (*it)->bindToApplication();
    }
}

QDBUS_EXPORT void qDBusBindToApplication();
void qDBusBindToApplication()
{
    _q_manager()->bindToApplication();
}

void QDBusConnectionManager::setConnection(const QString &name, QDBusConnectionPrivate *c)
{
    connectionHash[name] = c;
    c->name = name;
}

/*!
    \fn QDBusConnection &QDBusConnection::sessionBus()
    \relates QDBusConnection

    Returns a QDBusConnection object opened with the session bus. The object reference returned
    by this function is valid until the QCoreApplication's destructor is run, when the
    connection will be closed and the object, deleted.
*/
/*!
    \fn QDBusConnection &QDBusConnection::systemBus()
    \relates QDBusConnection

    Returns a QDBusConnection object opened with the system bus. The object reference returned
    by this function is valid until the QCoreApplication's destructor is run, when the
    connection will be closed and the object, deleted.
*/

/*!
    \class QDBusConnection
    \inmodule QtDBus
    \since 4.2

    \brief The QDBusConnection class represents a connection to the D-Bus bus daemon.

    This class is the initial point in a D-Bus session. Using it, you
    can get access to remote objects, interfaces; connect remote
    signals to your object's slots; register objects, etc.

    D-Bus connections are created using the connectToBus() function,
    which opens a connection to the server daemon and does the initial
    handshaking, associating that connection with a name. Further
    attempts to connect using the same name will return the same
    connection.

    The connection is then torn down using the disconnectFromBus()
    function.

    As a convenience for the two most common connection types, the
    sessionBus() and systemBus() functions return open connections to
    the session server daemon and the system server daemon,
    respectively. Those connections are opened when first used and are
    closed when the QCoreApplication destructor is run.

    D-Bus also supports peer-to-peer connections, without the need for
    a bus server daemon. Using this facility, two applications can
    talk to each other and exchange messages. This can be achieved by
    passing an address to connectToBus() function, which was opened by
    another D-Bus application using QDBusServer.
*/

/*!
    \enum QDBusConnection::BusType
    Specifies the type of the bus connection. The valid bus types are:

    \value SessionBus           the session bus, associated with the running desktop session
    \value SystemBus            the system bus, used to communicate with system-wide processes
    \value ActivationBus        the activation bus, the "alias" for the bus that started the
                                service

    On the Session Bus, one can find other applications by the same user that are sharing the same
    desktop session (hence the name). On the System Bus, however, processes shared for the whole
    system are usually found.
*/

/*!
    \enum QDBusConnection::RegisterOption
    Specifies the options for registering objects with the connection. The possible values are:

    \value ExportAdaptors                       export the contents of adaptors found in this object

    \value ExportScriptableSlots                export this object's scriptable slots
    \value ExportScriptableSignals              export this object's scriptable signals
    \value ExportScriptableProperties           export this object's scriptable properties
    \value ExportScriptableContents             shorthand form for ExportScriptableSlots |
                                                ExportScriptableSignals |
                                                ExportScriptableProperties

    \value ExportNonScriptableSlots             export this object's non-scriptable slots
    \value ExportNonScriptableSignals           export this object's non-scriptable signals
    \value ExportNonScriptableProperties        export this object's non-scriptable properties
    \value ExportNonScriptableContents          shorthand form for ExportNonScriptableSlots |
                                                ExportNonScriptableSignals |
                                                ExportNonScriptableProperties

    \value ExportAllSlots                       export all of this object's slots
    \value ExportAllSignal                      export all of this object's signals
    \value ExportAllProperties                  export all of this object's properties
    \value ExportAllContents                    export all of this object's contents

    \value ExportChildObjects                   export this object's child objects

    \warning It is currently not possible to export signals from
    objects. If you pass the ExportScriptableSignals,
    ExportNonScriptableSignals, or ExportAllSignal, the
    registerObject() function will print a warning.

    \sa registerObject(), QDBusAbstractAdaptor, {usingadaptors.html}{Using adaptors}
*/

/*!
    \enum QDBusConnection::UnregisterMode
    The mode for unregistering an object path:

    \value UnregisterNode       unregister this node only: do not unregister child objects
    \value UnregisterTree       unregister this node and all its sub-tree

    Note, however, if this object was registered with the ExportChildObjects option, UnregisterNode
    will unregister the child objects too.
*/

/*!
    Creates a QDBusConnection object attached to the connection with name \a name.

    This does not open the connection. You have to call connectToBus() to open it.
*/
QDBusConnection::QDBusConnection(const QString &name)
{
    if (name.isEmpty()) {
        d = 0;
    } else {
        d = _q_manager()->connection(name);
        if (d)
            d->ref.ref();
    }
}

/*!
    Creates a copy of the \a other connection.
*/
QDBusConnection::QDBusConnection(const QDBusConnection &other)
{
    d = other.d;
    if (d)
        d->ref.ref();
}

/*!
  \internal
   Creates a connection object with the given \a dd as private object.
*/
QDBusConnection::QDBusConnection(QDBusConnectionPrivate *dd)
{
    d = dd;
    if (d)
        d->ref.ref();
}

/*!
    Disposes of this object. This does not close the connection: you
    have to call disconnectFromBus() to do that.
*/
QDBusConnection::~QDBusConnection()
{
    if (d && !d->ref.deref())
        delete d;
}

/*!
    Creates a copy of the connection \a other in this object. Note
    that the connection this object referenced before the copy, is not
    spontaneously disconnected.

    \sa disconnectFromBus()
*/
QDBusConnection &QDBusConnection::operator=(const QDBusConnection &other)
{
    if (other.d)
        other.d->ref.ref();
    QDBusConnectionPrivate *old = static_cast<QDBusConnectionPrivate *>(
            q_atomic_set_ptr(&d, other.d));
    if (old && !old->ref.deref())
        delete old;

    return *this;
}

/*!
    Opens a connection of type \a type to one of the known busses and
    associate with it the connection name \a name. Returns a
    QDBusConnection object associated with that connection.
*/
QDBusConnection QDBusConnection::connectToBus(BusType type, const QString &name)
{
//    Q_ASSERT_X(QCoreApplication::instance(), "QDBusConnection::addConnection",
//               "Cannot create connection without a Q[Core]Application instance");

    QDBusConnectionPrivate *d = _q_manager()->connection(name);
    if (d || name.isEmpty())
        return QDBusConnection(name);

    d = new QDBusConnectionPrivate;
    DBusConnection *c = 0;
    switch (type) {
        case SystemBus:
            c = dbus_bus_get_private(DBUS_BUS_SYSTEM, &d->error);
            break;
        case SessionBus:
            c = dbus_bus_get_private(DBUS_BUS_SESSION, &d->error);
            break;
        case ActivationBus:
            c = dbus_bus_get_private(DBUS_BUS_STARTER, &d->error);
            break;
    }
    d->setConnection(c); //setConnection does the error handling for us

    _q_manager()->setConnection(name, d);

    QDBusConnection retval(name);

    // create the bus service
    d->busService = new QDBusConnectionInterface(retval, d);
    d->ref.deref();              // busService has a increased the refcounting to us
                                 // avoid cyclic refcounting

    QObject::connect(d->busService, SIGNAL(serviceOwnerChanged(QString,QString,QString)),
                     d, SIGNAL(serviceOwnerChanged(QString,QString,QString)));

    return retval;
}

/*!
    Opens a peer-to-peer connection on address \a address and associate with it the
    connection name \a name. Returns a QDBusConnection object associated with that connection.
*/
QDBusConnection QDBusConnection::connectToBus(const QString &address,
                                              const QString &name)
{
//    Q_ASSERT_X(QCoreApplication::instance(), "QDBusConnection::addConnection",
//               "Cannot create connection without a Q[Core]Application instance");

    QDBusConnectionPrivate *d = _q_manager()->connection(name);
    if (d || name.isEmpty())
        return QDBusConnection(name);

    d = new QDBusConnectionPrivate;
    // setConnection does the error handling for us
    d->setConnection(dbus_connection_open(address.toUtf8().constData(), &d->error));

    _q_manager()->setConnection(name, d);

    QDBusConnection retval(name);

    // create the bus service
    // create the bus service
    d->busService = new QDBusConnectionInterface(retval, d);
    d->ref.deref();              // busService has a increased the refcounting to us
                                 // avoid cyclic refcounting
    QObject::connect(d->busService, SIGNAL(serviceOwnerChanged(QString,QString,QString)),
                     d, SIGNAL(serviceOwnerChanged(QString,QString,QString)));

    return retval;
}

/*!
    Closes the connection of name \a name.

    Note that if there are still QDBusConnection objects associated
    with the same connection, the connection will not be closed until
    all references are dropped. However, no further references can be
    created using the QDBusConnection constructor.
*/
void QDBusConnection::disconnectFromBus(const QString &name)
{
    if (_q_manager())
        _q_manager()->removeConnection(name);
}

/*!
    Sends the \a message over this connection, without waiting for a
    reply. This is suitable for errors, signals, and return values as
    well as calls whose return values are not necessary.

    Returns true if the message was queued successfully, false otherwise.
*/
bool QDBusConnection::send(const QDBusMessage &message) const
{
    if (!d || !d->connection) {
        QDBusError err = QDBusError(QDBusError::Disconnected,
                                    QLatin1String("Not connected to D-BUS server"));
        if (d)
            d->lastError = err;

        return false;
    }
    return d->send(message) != 0;
}

/*!
    Sends the \a message over this connection and blocks, waiting for
    a reply, for at most \a timeout milliseconds.  When the reply is
    received, the given \a method is called in the \a receiver object.
    The default \a timeout is -1, meaning that the function won't wait
    for a reply before calling the specified \a method.

    This function is suitable for method calls only. It is guaranteed
    that the slot will be called exactly once with the reply, as long
    as the parameter types match. If they don't, the reply cannot be
    delivered.

    Returns the identification of the message that was sent or 0 if
    nothing was sent.

*/
bool QDBusConnection::callWithCallback(const QDBusMessage &message, QObject *receiver,
                                       const char *method, int timeout) const
{
    if (!d || !d->connection) {
        QDBusError err = QDBusError(QDBusError::Disconnected,
                                    QLatin1String("Not connected to D-BUS server"));
        if (d)
            d->lastError = err;

        return 0;
    }

    return d->sendWithReplyAsync(message, receiver, method, timeout) != 0;
}

/*!
    Sends the \a message over this connection and blocks, waiting for
    a reply, for at most \a timeout milliseconds. This function is
    suitable for method calls only. It returns the reply message as
    its return value, which will be either of type
    QDBusMessage::ReplyMessage or QDBusMessage::ErrorMessage.

    See the QDBusInterface::call() function for a more friendly way
    of placing calls.

    \warning If \a mode is QDBus::BlockWithGui, this function will
             reenter the Qt event loop in order to wait for the
             reply. During the wait, it may deliver signals and other
             method calls to your application. Therefore, it must be
             prepared to handle a reentrancy whenever a call is
             placed with call().
*/
QDBusMessage QDBusConnection::call(const QDBusMessage &message, QDBus::CallMode mode, int timeout) const
{
    if (!d || !d->connection) {
        QDBusError err = QDBusError(QDBusError::Disconnected,
                                    QLatin1String("Not connected to D-BUS server"));
        if (d)
            d->lastError = err;

        return QDBusMessagePrivate::fromError(err);
    }

    if (mode != QDBus::NoBlock)
        return d->sendWithReply(message, mode, timeout);

    d->send(message);
    QDBusMessage retval;
    retval << QVariant(); // add one argument (to avoid .at(0) problems)
    return retval;
}

/*!
    Connects the signal specified by the \a service, \a path, \a interface and \a name parameters to
    the slot \a slot in object \a receiver. The arguments \a service and \a path can be empty,
    denoting a connection to any signal of the (\a interface, \a name) pair, from any remote
    application.

    Returns true if the connection was successful.

    \warning The signal will only be delivered to the slot if the parameters match. This verification
             can be done only when the signal is received, not at connection time.
*/
bool QDBusConnection::connect(const QString &service, const QString &path, const QString& interface,
                              const QString &name, QObject *receiver, const char *slot)
{
    return connect(service, path, interface, name, QString(), receiver, slot);
}

/*!
    Disconnects the signal specified by the \a service, \a path, \a interface and \a name parameters from
    the slot \a slot in object \a receiver. The arguments \a service and \a path can be empty,
    denoting a disconnection from all signals of the (\a interface, \a name) pair, from all remote
    applications.

    Returns true if the disconnection was successful.
*/
bool QDBusConnection::disconnect(const QString &service, const QString &path, const QString &interface,
                                 const QString &name, QObject *receiver, const char *slot)
{
    return disconnect(service, path, interface, name, QString(), receiver, slot);
}

/*!
    \overload

    Connects the signal to the slot \a slot in object \a
    receiver. Unlike the other connect() overload, this function
    allows one to specify the parameter signature to be connected
    using the \a signature variable. The function will then verify
    that this signature can be delivered to the slot specified by \a
    slot and return false otherwise.
*/
bool QDBusConnection::connect(const QString &service, const QString &path, const QString& interface,
                              const QString &name, const QString &signature,
                              QObject *receiver, const char *slot)
{
    if (!receiver || !slot || !d || !d->connection)
        return false;
    if (!interface.isEmpty() && !QDBusUtil::isValidInterfaceName(interface))
        return false;
    if (interface.isEmpty() && name.isEmpty())
        return false;

    // check the slot
    QDBusConnectionPrivate::SignalHook hook;
    QString key;
    QString name2 = name;
    if (name2.isNull())
        name2.detach();

    QString owner = d->getNameOwner(service); // we don't care if the owner is empty
    hook.signature = signature;               // it might get started later
    if (!d->prepareHook(hook, key, service, owner, path, interface, name, receiver, slot, 0, false))
        return false;           // don't connect

    // avoid duplicating:
    QWriteLocker locker(&d->lock);
    QDBusConnectionPrivate::SignalHookHash::ConstIterator it = d->signalHooks.find(key);
    QDBusConnectionPrivate::SignalHookHash::ConstIterator end = d->signalHooks.constEnd();
    for ( ; it != end && it.key() == key; ++it) {
        const QDBusConnectionPrivate::SignalHook &entry = it.value();
        if (entry.service == hook.service &&
            entry.owner == hook.owner &&
            entry.path == hook.path &&
            entry.signature == hook.signature &&
            entry.obj == hook.obj &&
            entry.midx == hook.midx) {
            // no need to compare the parameters if it's the same slot
            return true;        // already there
        }
    }

    d->connectSignal(key, hook);
    return true;
}

/*!
    \overload

    Disconnects the signal from the slot \a slot in object \a
    receiver. Unlike the other disconnect() overload, this function
    allows one to specify the parameter signature to be disconnected
    using the \a signature variable. The function will then verify
    that this signature is connected to the slot specified by \a slot
    and return false otherwise.
*/
bool QDBusConnection::disconnect(const QString &service, const QString &path, const QString& interface,
                                 const QString &name, const QString &signature,
                                 QObject *receiver, const char *slot)
{
    if (!receiver || !slot || !d || !d->connection)
        return false;
    if (!interface.isEmpty() && !QDBusUtil::isValidInterfaceName(interface))
        return false;
    if (interface.isEmpty() && name.isEmpty())
        return false;

    // check the slot
    QDBusConnectionPrivate::SignalHook hook;
    QString key;
    QString name2 = name;
    if (name2.isNull())
        name2.detach();

    QString owner = d->getNameOwner(service); // we don't care of owner is empty
    hook.signature = signature;
    if (!d->prepareHook(hook, key, service, owner, path, interface, name, receiver, slot, 0, false))
        return false;           // don't disconnect

    // avoid duplicating:
    QWriteLocker locker(&d->lock);
    QDBusConnectionPrivate::SignalHookHash::ConstIterator it = d->signalHooks.find(key);
    QDBusConnectionPrivate::SignalHookHash::ConstIterator end = d->signalHooks.constEnd();
    for ( ; it != end && it.key() == key; ++it) {
        const QDBusConnectionPrivate::SignalHook &entry = it.value();
        if (entry.service == hook.service &&
            entry.owner == hook.owner &&
            entry.path == hook.path &&
            entry.signature == hook.signature &&
            entry.obj == hook.obj &&
            entry.midx == hook.midx) {
            // no need to compare the parameters if it's the same slot
            d->disconnectSignal(key, hook);
            return true;        // it was there
        }
    }

    // the slot was not found
    return false;
}

/*!
    Registers the object \a object at path \a path and returns true if
    the registration was successful. The \a options parameter
    specifies how much of the object \a object will be exposed through
    D-Bus.

    This function does not replace existing objects: if there is already an object registered at
    path \a path, this function will return false. Use unregisterObject() to unregister it first.

    You cannot register an object as a child object of an object that
    was registered with QDBusConnection::ExportChildObjects.
*/
bool QDBusConnection::registerObject(const QString &path, QObject *object, RegisterOptions options)
{
    Q_ASSERT_X(QDBusUtil::isValidObjectPath(path), "QDBusConnection::registerObject",
               "Invalid object path given");
    if (!d || !d->connection || !object || !options || !QDBusUtil::isValidObjectPath(path))
        return false;

    QStringList pathComponents = path.split(QLatin1Char('/'));
    if (pathComponents.last().isEmpty())
        pathComponents.removeLast();
    QWriteLocker locker(&d->lock);

    // lower-bound search for where this object should enter in the tree
    QDBusConnectionPrivate::ObjectTreeNode *node = &d->rootNode;
    int i = 1;
    while (node) {
        if (pathComponents.count() == i) {
            // this node exists
            // consider it free if there's no object here and the user is not trying to
            // replace the object sub-tree
            if ((options & ExportChildObjects && !node->children.isEmpty()) || node->obj)
                return false;

            // we can add the object here
            node->obj = object;
            node->flags = options;

            d->registerObject(node);
            //qDebug("REGISTERED FOR %s", path.toLocal8Bit().constData());
            return true;
        }

        // find the position where we'd insert the node
        QVector<QDBusConnectionPrivate::ObjectTreeNode::Data>::Iterator it =
            qLowerBound(node->children.begin(), node->children.end(), pathComponents.at(i));
        if (it != node->children.constEnd() && it->name == pathComponents.at(i)) {
            // match: this node exists
            node = it->node;

            // are we allowed to go deeper?
            if (node->flags & ExportChildObjects) {
                // we're not
                qDebug("Cannot register object at %s because %s exports its own child objects",
                       qPrintable(path), qPrintable(pathComponents.at(i)));
                return false;
            }
        } else {
            // add entry
            QDBusConnectionPrivate::ObjectTreeNode::Data entry;
            entry.name = pathComponents.at(i);
            entry.node = new QDBusConnectionPrivate::ObjectTreeNode;
            node->children.insert(it, entry);

            node = entry.node;
        }

        // iterate
        ++i;
    }

    Q_ASSERT_X(false, "QDBusConnection::registerObject", "The impossible happened");
    return false;
}

/*!
    Unregisters an object that was registered with the registerObject() at the object path given by
    \a path and, if \a mode is QDBusConnection::UnregisterTree, all of its sub-objects too.

    Note that you cannot unregister objects that were not registered with registerObject().
*/
void QDBusConnection::unregisterObject(const QString &path, UnregisterMode mode)
{
    if (!d || !d->connection || !QDBusUtil::isValidObjectPath(path))
        return;

    QStringList pathComponents = path.split(QLatin1Char('/'));
    QWriteLocker locker(&d->lock);
    QDBusConnectionPrivate::ObjectTreeNode *node = &d->rootNode;
    int i = 1;

    // find the object
    while (node) {
        if (pathComponents.count() == i) {
            // found it
            node->obj = 0;
            node->flags = 0;

            if (mode == UnregisterTree) {
                // clear the sub-tree as well
                node->clear();  // can't disconnect the objects because we really don't know if they can
                                // be found somewhere else in the path too
            }

            return;
        }

        QVector<QDBusConnectionPrivate::ObjectTreeNode::Data>::ConstIterator it =
            qLowerBound(node->children.constBegin(), node->children.constEnd(), pathComponents.at(i));
        if (it == node->children.constEnd() || it->name != pathComponents.at(i))
            break;              // node not found

        node = it->node;
        ++i;
    }
}

/*!
    Return the object that was registered with the registerObject() at the object path given by
    \a path.
*/
QObject *QDBusConnection::objectRegisteredAt(const QString &path) const
{
    Q_ASSERT_X(QDBusUtil::isValidObjectPath(path), "QDBusConnection::registeredObject",
               "Invalid object path given");
    if (!d || !d->connection || !QDBusUtil::isValidObjectPath(path))
        return false;

    QStringList pathComponents = path.split(QLatin1Char('/'));
    if (pathComponents.last().isEmpty())
        pathComponents.removeLast();

    // lower-bound search for where this object should enter in the tree
    QDBusConnectionPrivate::ObjectTreeNode *node = &d->rootNode;

    int i = 1;
    while (node) {
        if (pathComponents.count() == i)
            return node->obj;

        QVector<QDBusConnectionPrivate::ObjectTreeNode::Data>::ConstIterator it =
            qLowerBound(node->children.constBegin(), node->children.constEnd(), pathComponents.at(i));
        if (it == node->children.constEnd() || it->name != pathComponents.at(i))
            break;              // node not found

        node = it->node;
        ++i;
    }
    return 0;
}

/*!
    Returns a QDBusConnectionInterface object that represents the
    D-BUS server interface on this connection.
*/
QDBusConnectionInterface *QDBusConnection::interface() const
{
    if (!d)
        return 0;
    return d->busService;
}

/*!
    Returns true if this QDBusConnection object is connected.

    If it isn't connected, calling connectToBus() on the same
    connection name will not make be connected. You need to call the
    QDBusConnection constructor again.
*/
bool QDBusConnection::isConnected() const
{
    return d && d->connection && dbus_connection_get_is_connected(d->connection);
}

/*!
    Returns the last error that happened in this connection.

    This function is provided for low-level code. If you're using
    QDBusInterface::call(), error codes are reported by its return
    value.

    \sa QDBusInterface, QDBusMessage
*/
QDBusError QDBusConnection::lastError() const
{
    return d ? d->lastError : QDBusError();
}

/*!
    Returns the unique connection name for this connection, if this QDBusConnection object is
    connected, or an empty QString otherwise.

    A Unique Connection Name is a string in the form ":x.xxx" (where x
    are decimal digits) that is assigned by the D-Bus server daemon
    upon connection. It uniquely identifies this client in the bus.

    This function returns an empty QString for peer-to-peer connections.
*/
QString QDBusConnection::baseService() const
{
    return d->baseService();
}

/*!
    Attempts to register the \a serviceName on the D-BUS server and
    returns true if the registration succeded. The registration will
    fail if the name is already registered by another application.

    \sa unregisterService(), QDBusConnectionInterface::registerService()
*/
bool QDBusConnection::registerService(const QString &serviceName)
{
    if (d) d->registerService(serviceName);
    return interface()->registerService(serviceName);
}

/*!
    Unregisters the service \a serviceName that was previously
    registered with registerService() and returns true if it
    succeeded.

    \sa registerService(), QDBusConnectionInterface::unregisterService()
*/
bool QDBusConnection::unregisterService(const QString &serviceName)
{
    if (d) d->unregisterService(serviceName);
    return interface()->unregisterService(serviceName);
}

static const char _q_sessionBusName[] = "qt_default_session_bus";
static const char _q_systemBusName[] = "qt_default_system_bus";

class QDBusDefaultConnection: public QDBusConnection
{
    const char *ownName;
public:
    inline QDBusDefaultConnection(BusType type, const char *name)
        : QDBusConnection(connectToBus(type, QString::fromLatin1(name))), ownName(name)
    { }

    inline ~QDBusDefaultConnection()
    { disconnectFromBus(QString::fromLatin1(ownName)); }
};

Q_GLOBAL_STATIC_WITH_ARGS(QDBusDefaultConnection, _q_sessionBus,
                          (QDBusConnection::SessionBus, _q_sessionBusName))
Q_GLOBAL_STATIC_WITH_ARGS(QDBusDefaultConnection, _q_systemBus,
                          (QDBusConnection::SystemBus, _q_systemBusName))

QDBusConnection QDBusConnection::sessionBus()
{
    return *_q_sessionBus();
}

QDBusConnection QDBusConnection::systemBus()
{
    return *_q_systemBus();
}

/*!
  Returns the connection that sent the signal, if called in a slot activated
  by QDBus; otherwise it returns 0.
*/
QDBusConnection QDBusConnection::sender()
{
    return QDBusConnection(_q_manager()->sender());
}

/*!
  \internal
*/
void QDBusConnectionPrivate::setSender(const QDBusConnectionPrivate *s)
{
    _q_manager()->setSender(s);
}

/*!
    \namespace QDBus
    \inmodule QtDBus

    \brief The QDBus namespace contains miscellaneous identifiers used
    throughout the QtDBus library.
*/

/*!
    \enum QDBus::CallMode

    This enum describes the various ways of placing a function call. The valid modes are:

    \value NoBlock              Place the call but don't wait for the reply (the reply's contents
                                will be discarded).
    \value Block                Don't use an event loop to wait for a reply, but instead block on
                                network operations while waiting. This means the
                                user-interface may not be updated until the function returns.
    \value BlockWithGui         Use the Qt event loop to wait for a reply. This means that the
                                user-interface will stay responsive (processing input events),
                                but it also means other events may happen, like signal delivery
                                and other D-Bus method calls.
    \value AutoDetect           Automatically detect if the called function has a reply.

    When using BlockWithGui, applications must be prepared for reentrancy in any function.
*/

