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

#include <qtopiacomm/private/qbluetoothnamespace_p.h>
#include <qbluetoothpasskeyrequest.h>
#include <qbluetoothpasskeyagent.h>
#include <qtopialog.h>

#include <QList>
#include <qglobal.h>

#include <qtdbus/qdbusargument.h>
#include <qtdbus/qdbusconnection.h>
#include <qtdbus/qdbusinterface.h>
#include <qtdbus/qdbusreply.h>
#include <qtdbus/qdbusmessage.h>
#include <qtdbus/qdbusabstractadaptor.h>

#include <stdio.h>
#include <string.h>

#include <QDebug>

class PasskeyAgentDBusAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.PasskeyAgent")

public:
    PasskeyAgentDBusAdaptor(QObject *parent);
    ~PasskeyAgentDBusAdaptor();
};

PasskeyAgentDBusAdaptor::PasskeyAgentDBusAdaptor(QObject *parent) :
         QDBusAbstractAdaptor(parent)
{

}

PasskeyAgentDBusAdaptor::~PasskeyAgentDBusAdaptor()
{

}

class QBluetoothPasskeyAgent_Private : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.PasskeyAgent")

public:
    QBluetoothPasskeyAgent_Private(QBluetoothPasskeyAgent *parent,
                                   const QString &name);
    QString m_name;
    QBluetoothPasskeyAgent *m_parent;
    QBluetoothPasskeyAgent::Error m_error;

    bool registerAgent(const QString &localAdapter, const QString &addr);
    bool unregisterAgent(const QString &localAdapter, const QString &addr);
    void handleError(const QDBusError &error);

public slots:
    void Request(const QString &path,
                 const QString &address,
                 const QDBusMessage &msg);
    void Cancel(const QString &path, const QString &address);
    void Release();
};

QBluetoothPasskeyAgent_Private::QBluetoothPasskeyAgent_Private(QBluetoothPasskeyAgent *parent, const QString &name) : QObject(parent)
{
    m_parent = parent;
    m_name = name;
    m_error = QBluetoothPasskeyAgent::NoError;

    QDBusConnection dbc = QDBusConnection::systemBus();
    if (!dbc.isConnected()) {
        qWarning() << "Unable to connect do D-BUS:" << dbc.lastError();
        return;
    }


    QString path = m_name;
    path.prepend('/');

    new PasskeyAgentDBusAdaptor(this);
    dbc.registerObject(path, this, QDBusConnection::ExportNonScriptableSlots);
}

struct bluez_error_mapping
{
    const char *name;
    QBluetoothPasskeyAgent::Error error;
};

static bluez_error_mapping bluez_errors[] = {
    { "org.bluez.Error.AlreadyExists", QBluetoothPasskeyAgent::AlreadyExists },
    { "org.bluez.Error.DoesNotExist", QBluetoothPasskeyAgent::DoesNotExist },
    { "org.bluez.Error.UnknownAddress", QBluetoothPasskeyAgent::UnknownAddress },
    { NULL, QBluetoothPasskeyAgent::NoError }
};

void QBluetoothPasskeyAgent_Private::handleError(const QDBusError &error)
{
    m_error = QBluetoothPasskeyAgent::UnknownError;
    qLog(Bluetooth) << "Decoding error:" << error;

    int i = 0;
    while (bluez_errors[i].name) {
        if (error.name() == bluez_errors[i].name) {
            m_error = bluez_errors[i].error;
            break;
        }
        i++;
    }
}

void QBluetoothPasskeyAgent_Private::Request(const QString &path,
                                             const QString &address,
                                             const QDBusMessage &msg)
{
    QString devname = path.mid(11);
    QBluetoothAddress addr(address);

    QBluetoothPasskeyRequest req(devname, addr);

    m_parent->requestPasskey(req);

    QDBusMessage reply;
    if (req.isRejected()) {
        reply = msg.createErrorReply("org.bluez.Error.Rejected", "Rejected");
    }
    else {
        reply = msg.createReply(req.passkey());
    }

    QDBusConnection::systemBus().send(reply);
}

void QBluetoothPasskeyAgent_Private::Cancel(const QString &path, const QString &address)
{
    QBluetoothAddress addr( address);
    m_parent->cancelRequest(path.mid(11), addr);
}

void QBluetoothPasskeyAgent_Private::Release()
{
    m_parent->release();
}

bool QBluetoothPasskeyAgent_Private::registerAgent(const QString &localAdapter,
        const QString &addr)
{
    QString bluezAdapter = "/org/bluez";

    if (!localAdapter.isNull()) {
        bluezAdapter.append("/");
        bluezAdapter.append(localAdapter);
    }

    QDBusInterface *iface = new QDBusInterface("org.bluez",
                                               bluezAdapter,
                                               "org.bluez.Security",
                                               QDBusConnection::systemBus());

    if (!iface->isValid())
        return false;

    QString bluezMethod;
    QVariantList args;

    QString path = m_name;
    path.prepend('/');
    args << path;

    if (addr.isNull()) {
        bluezMethod = "RegisterDefaultPasskeyAgent";
    }
    else {
        bluezMethod = "RegisterPasskeyAgent";
        args << addr;
    }

    QDBusReply<void> reply = iface->callWithArgumentList(QDBus::Block,
            bluezMethod, args);

    if (!reply.isValid()) {
        handleError(reply.error());
        return false;
    }

    return true;
}

bool QBluetoothPasskeyAgent_Private::unregisterAgent(const QString &localAdapter,
        const QString &addr)
{
    QString bluezAdapter = "/org/bluez";

    if (!localAdapter.isNull()) {
        bluezAdapter.append("/");
        bluezAdapter.append(localAdapter);
    }

    QDBusInterface *iface = new QDBusInterface("org.bluez",
                                               bluezAdapter,
                                               "org.bluez.Security",
                                               QDBusConnection::systemBus());

    if (!iface->isValid())
        return false;

    QString bluezMethod;
    QVariantList args;

    QString path = m_name;
    path.prepend('/');
    args << path;

    if (addr.isNull()) {
        bluezMethod = "UnregisterDefaultPasskeyAgent";
    }
    else {
        bluezMethod = "UnregisterPasskeyAgent";
        args << addr;
    }

    QDBusReply<void> reply = iface->callWithArgumentList(QDBus::Block,
            bluezMethod, args);

    if (!reply.isValid()) {
        handleError(reply.error());
        return false;
    }

    return true;
}

/*!
    \class QBluetoothPasskeyAgent
    \brief The QBluetoothPasskeyAgent class represents a passkey entry proxy.

    The QBluetoothPasskeyAgent class provides an abstract interface for
    requesting and providing passkeys to the bluetooth system whenever a new
    pairing procedure is initiated.  It is up to the clients to actually
    establish how the passkey is obtained.  For instance, it could ask the
    user, or read passkeys from a file, or in some other fashion.

    The passkey agent can be registered as a global default, a default for a
    particular local bluetooth adapter, or just for a specific remote device.

    \ingroup qtopiabluetooth
    \sa QBluetoothAddress, QBluetoothRemoteDevice, QBluetoothLocalDevice
 */

/*!
    \enum QBluetoothPasskeyAgent::Error
    \brief Possible errors that might occur.

    \value NoError No error.
    \value AlreadyExists Another passkey agent has already been registered.
    \value DoesNotExist The passkey agent has not been registered.
    \value UnknownAddress The address of the remote device is unknown.
    \value UnknownError An unknown error has occurred.
*/

/*!
    Constructs a passkey agent.  The \a name parameter specifies a unique name
    of the passkey agent.  The name should be a valid identifier name, e.g.
    it cannot contain special characters or start with a number.
    For instance: DefaultPasskeyAgent.

    The \a parent parameter holds the QObject parent.
*/
QBluetoothPasskeyAgent::QBluetoothPasskeyAgent(const QString &name, QObject *parent)
    : QObject(parent)
{
    m_data = new QBluetoothPasskeyAgent_Private(this, name);
}

/*!
    Destructor.
*/
QBluetoothPasskeyAgent::~QBluetoothPasskeyAgent()
{
    delete m_data;
}

/*!
    Returns the name of the passkey agent as a string.
*/
QString QBluetoothPasskeyAgent::name() const
{
    return m_data->m_name;
}

/*!
    Returns the last error that has occurred.
*/
QBluetoothPasskeyAgent::Error QBluetoothPasskeyAgent::lastError() const
{
    return m_data->m_error;
}

/*!
    This virtual function will be called whenever the bluetooth system
    has received a request for a passkey, and the agent is registered to
    handle the particular request.

    The \a request parameter contains the passkey request.
*/
void QBluetoothPasskeyAgent::requestPasskey(QBluetoothPasskeyRequest &request)
{
    Q_UNUSED(request)
}

/*!
    This function will be called whenever a passkey authentication request
    has failed.

    The request being cancelled is on \a localDevice and the address of the
    paired device is \a remoteAddr
*/
void QBluetoothPasskeyAgent::cancelRequest(const QString &localDevice, const QBluetoothAddress &remoteAddr)
{
    Q_UNUSED(localDevice)
    Q_UNUSED(remoteAddr)
}

/*!
    This function will be called whenever a passkey agent has been
    unregistered.

    \sa unregisterDefault(), unregisterForAddress()
*/
void QBluetoothPasskeyAgent::release()
{

}

/*!
    Register the passkey agent as the default agent for all local devices.
*/
bool QBluetoothPasskeyAgent::registerDefault()
{
    return m_data->registerAgent(QString(), QString());
}

/*!
    Unregister the passkey agent as the default agent for all local devices.
*/
bool QBluetoothPasskeyAgent::unregisterDefault()
{
    return m_data->unregisterAgent(QString(), QString());
}

/*!
    Register the passkey agent as the default agent for device given
    by \a localDevice.
*/
bool QBluetoothPasskeyAgent::registerDefault(const QString &localDevice)
{
    return m_data->registerAgent(localDevice, QString());
}

/*!
    Unregister the passkey agent as the default agent for device given
    by \a localDevice.
 */
bool QBluetoothPasskeyAgent::unregisterDefault(const QString &localDevice)
{
    return m_data->unregisterAgent(localDevice, QString());
}

/*!
    Register the passkey agent for all local devices. It will only handle
    pairing requests associated with remote device which is given in
    \a addr.

    Note that once pairing is complete, or a timeout has been
    hit, the agent will be automatically unregistered.  The \c release()
    method will be called.  It is up to the application to register the
    agent again.
 */
bool QBluetoothPasskeyAgent::registerForAddress(QBluetoothAddress &addr)
{
    return m_data->registerAgent(QString(), addr.toString());
}

/*!
    Unregister the passkey agent for all local devices. This method
    attempts to unregister an agent that would have only handled
    pairing requests associated with remote device at address \a addr.
*/
bool QBluetoothPasskeyAgent::unregisterForAddress(QBluetoothAddress &addr)
{
    return m_data->unregisterAgent(QString(), addr.toString());
}

/*!
    Register the passkey agent for local device represented by
    \a localDevice. It will only handle pairing requests associated
    with remote device which is given in \a addr.

    Note that once pairing is complete, or a timeout has been
    hit, the agent will be automatically unregistered.  The \c release()
    method will be called.  It is up to the application to register the
    agent again.
 */
bool QBluetoothPasskeyAgent::registerForAddress(const QString &localDevice,
         QBluetoothAddress &addr)
{
    return m_data->registerAgent(localDevice, addr.toString());
}

/*!
    Unregister the passkey agent for local device \a localDevice. This method
    attempts to unregister an agent that would have only handled
    pairing requests associated with remote device at address \a addr.
 */
bool QBluetoothPasskeyAgent::unregisterForAddress(const QString &localDevice,
         QBluetoothAddress &addr)
{
    return m_data->unregisterAgent(localDevice, addr.toString());
}

#include "qbluetoothpasskeyagent.moc"
