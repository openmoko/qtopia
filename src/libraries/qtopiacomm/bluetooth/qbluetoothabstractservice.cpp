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

#include "qbluetoothabstractservice.h"

#include <qbluetoothservicecontrol.h>
#include <qbluetoothaddress.h>
#include <qbluetoothlocaldevice.h>
#include <qsdap.h>
#include <qsdp.h>

#include <qtopiacomm/private/qbluetoothservicecontrolserver_p.h>

#include <qtopialog.h>

class QBluetoothAbstractService_Private : public QObject
{
    Q_OBJECT

public:
    QBluetoothAbstractService_Private(const QString &name, QBluetoothAbstractService *parent);
    ~QBluetoothAbstractService_Private();

    bool sdpRegister(const QBluetoothAddress &local,
                     QBluetooth::SDPProfile profile,
                     int channel);
    bool sdpUnregister();
    QString sdpError();

private slots:
    void searchResult(const QSDAPSearchResult &);

private:
    QBluetoothAbstractService *m_parent;
    QString m_name;
    QSDAP *m_sdap;
    QSDPService m_registeredService;
    QBluetoothAddress m_localAddr;
    int m_channel;
    bool m_searching;
    QString m_lastSdpError;
};

QBluetoothAbstractService_Private::QBluetoothAbstractService_Private(const QString &name, QBluetoothAbstractService *parent)
    : QObject(parent),
      m_parent(parent),
      m_name(name),
      m_sdap(new QSDAP()),
      m_searching(false)
{
    connect(m_sdap, SIGNAL(searchComplete(const QSDAPSearchResult &)),
            SLOT(searchResult(const QSDAPSearchResult &)));
}

QBluetoothAbstractService_Private::~QBluetoothAbstractService_Private()
{
    if (m_searching)
        m_sdap->cancelSearch();
    delete m_sdap;
    m_sdap = 0;
}


bool QBluetoothAbstractService_Private::sdpRegister(
                                            const QBluetoothAddress &local,
                                            QBluetooth::SDPProfile profile,
                                            int channel)
{
    m_localAddr = local;
    m_channel = channel;
    QBluetoothLocalDevice device(m_localAddr);

    if (!device.isValid()) {
        m_lastSdpError = "Cannot find local Bluetooth device";
        return false;
    }

    QSDP sdp;
    if (!sdp.registerService(device, profile, channel)) {
        m_lastSdpError = sdp.error();
        if (m_lastSdpError.isEmpty())
            m_lastSdpError = "Error registering with SDP server";
        return false;
    }

    // find registered service so it can be unregistered later
    m_sdap->searchLocalServices(device, profile);
    m_searching = true;
    return true;
}

bool QBluetoothAbstractService_Private::sdpUnregister()
{
    qLog(Bluetooth) << "QBluetoothAbstractService unregistering"
        << m_name << m_registeredService.recordHandle();

    QBluetoothLocalDevice device(m_localAddr);

    /* don't check local device, because you can unregister a service even
       if the local device is now invalid (since sdptool is separate)
    if (!device.isValid()) {
        m_lastSdpError = "Cannot find local Bluetooth device";
        return false;
    }
    */

    // what if m_registeredService hasn't been assigned?
    // unregisterService just returns false anyway?

    QSDP sdp;
    bool success = sdp.unregisterService(device, m_registeredService);
    if (success) {
        m_registeredService = QSDPService();    // reset
    } else {
        m_lastSdpError = sdp.error();
        if (m_lastSdpError.isEmpty())
            m_lastSdpError = "Error unregistering from SDP server";
    }
    return success;
}

QString QBluetoothAbstractService_Private::sdpError()
{
    return m_lastSdpError;
}

void QBluetoothAbstractService_Private::searchResult(const QSDAPSearchResult &result)
{
    const QList<QSDPService> &services = result.services();
    for (int i=0; i<services.size(); i++) {
        if (QSDPService::rfcommChannel(services[i]) == m_channel) {
            qLog(Bluetooth) << "Found registered service"
                << services[i].serviceName() << "with handle"
                << services[i].recordHandle();
            m_registeredService = services[i];
            m_searching = false;
            return;
        }
    }
}


/*!
    \class QBluetoothAbstractService
    \brief The QBluetoothAbstractService class is an abstract class for running a Bluetooth service within the Qtopia Bluetooth framework.

    The QBluetoothAbstractService class is an abstract class for handling the
    process of running a Bluetooth service within the Qtopia Bluetooth
    framework. Each subclass provides the specific implementation for
    running a particular Bluetooth service.

    This allows a custom Bluetooth service to be registered and accessible
    as a system Bluetooth service. For example, a subclass's associated
    service will be displayed in the system Bluetooth settings panel,
    allowing end users to modify the service's settings.

    The subclass should emit the provided signals as appropriate:
    \list
    \o emit started() when the service has started, or an error has occured while starting the service
    \o emit stopped() when the service has stopped, or an error has occured while starting the service
    \o emit error() when a fatal error has occured while running the service
    \endlist

    (Note that a \c QBluetoothAbstractService is very different from a
    Bluetooth SDP service that is registered through the \l QSDP class. For
    example, \c QBluetoothAbstractService could be subclassed to create a
    \c ObjectPushService class, and when start() is called, this subclass
    could advertise an Object Push SDP service (using the sdpRegister()
    function) and then start an Object Push server.)

    Here is an example of creating a custom Bluetooth service using a class
    named \c MyBluetoothService. The service will advertise the Serial
    Port Profile, and we'll call the service "MySerialPortService".

    Firstly, the constructor:

    \code
        MyBluetoothService::MyBluetoothService(QObject *parent = 0)
            : QBluetoothAbstractService("MySerialPortService", parent)
        {
            initialize();
        }
    \endcode

    This creates a service named "MySerialPortService". This name must be
    unique among Qtopia Bluetooth services, as it is used internally to
    uniquely identify the service (for example, for storing information in
    the value space). The code above also calls initialize() to perform the
    necessary set-up for internal components.

    Then, we implement the start() method:

    \code
        void MyBluetoothService::start(int channel)
        {
            if (!startMyService()) {    // call some method to start the service
                emit started(QBluetooth::UnknownError,
                          tr("Unable to start MySerialPortService"));
                return;
            }

            QBluetoothLocalDevice local;
            if (!sdpRegister(local.address(), QBluetooth::SerialPortProfile, channel)) {
                // call some method to shut down the service
                close();
                emit started(QBluetooth::SDPServerError,
                             tr("Error registering with SDP server"));
                return;
            }

            // service was successfully started
            emit started(QBluetooth::NoError, QString());
        }
    \endcode

    The start() method is called by Qtopia when the service should
    be started. Note how the example emits the started()
    signal when the service fails to start, and also when it does start
    successfully. Also, it calls sdpRegister() to advertise the
    appropriate SDP service.

    The class also needs to have a stop() method:

    \code
        void MyBluetoothService::stop()
        {
            // call some method to shut down the service
            close();

            if (!sdpUnregister()) {
                emit stopped(QBluetooth::SDPServerError,
                             tr("Error unregistering from SDP server"));
                return;
            }

            // service was successfully stopped
            emit stopped(QBluetooth::NoError, QString());
        }
    \endcode

    As with start(), the stopped() signal is emitted
    when the service is stopped successfully and also when it fails to stop.

    To set the display name for the service, implement
    translatableDisplayName():

    \code
        QString MyBluetoothService::translatableDisplayName() const
        {
            return tr("My Serial Port Service");
        }
    \endcode

    This string will be used, for example, to describe the service in the
    list of Bluetooth services in Qtopia's Bluetooth settings application.

    Finally, the class will need to implement the
    setSecurityOptions() method to provide the ability to modify
    the security settings for the service.

    \ingroup qtopiabluetooth
*/

/*!
    Construct a Bluetooth service. The \a name is a unique name that identifies
    the service. This name will also be used as the display name (e.g. in the
    BTSettings list of local services) unless translatableDisplayName() returns
    a different value. The \a parent is the QObject parent for this service.
 */
QBluetoothAbstractService::QBluetoothAbstractService(const QString &name, QObject *parent)
    : QAbstractIpcInterfaceGroup(name, parent),
      m_private(new QBluetoothAbstractService_Private(name, this))
{
}

/*!
    Deconstructs a Bluetooth service.
 */
QBluetoothAbstractService::~QBluetoothAbstractService()
{
}

/*!
    Initialize the service. This must be called to complete the initialization
    process.

    If the service is being created for the first time, when this method is
    called, the service will automatically be told to start (i.e. start() will
    be called). Otherwise, the service will only be started if it was still
    running when the system was last shut down.
 */
void QBluetoothAbstractService::initialize()
{
    if (!supports<QBluetoothServiceControl>()) {
        addInterface(new QBluetoothServiceControlServer(this, this));
    }

    QAbstractIpcInterfaceGroup::initialize();
}

/*!
    \fn void QBluetoothAbstractService::start(int channel)

    Starts this service on the RFCOMM channel \a channel.

    Subclasses must override this to start the service appropriately.

    A subclass must emit started() when the service has started (i.e. at the
    end of this method), or failed while trying to start.

    If the started() signal is emitted with QBluetooth::SDPServerError, the
    system will attempt to restart the service on a different channel.

    \warning This function must be implementated in such a way that any intermediate
    objects (which have be created up to the point where the error occured)
    are cleaned up before the error signal is emitted.

    \sa started()
 */

/*!
    \fn void QBluetoothAbstractService::stop()

    Stops this service.

    Subclasses must override this to stop the service appropriately.

    A subclass must emit stopped() when the service has stopped (i.e. at the end
    of this method), or failed while trying to stop.

    \warning An SDP error is not considered to be a critical error. The Bluetooth
    service manager will still assume that the service is stopped despite the fact
    that the sdp server couldn't unregister the service. This means the sdp unregistration
    must be the last step of this function.

    \sa stopped()
 */

/*!
    \fn void QBluetoothAbstractService::setSecurityOptions(QBluetooth::SecurityOptions options)

    Sets the security options for this service to the given \a options.

    Subclasses must override this to set the security options for this service.
 */

/*!
    \fn QString QBluetoothAbstractService::translatableDisplayName() const

    Returns the display name of this service. This will be used to
    display the service to the end user (e.g. in the BTSettings application).

    Subclasses must override this and return an appropriate name.
 */

 /*!
    Register the SDP service associated with this service. It will be registered
    on the local device with the given \a local address, using the given SDP
    profile \a profile and RFCOMM channel \a channel.
 */
bool QBluetoothAbstractService::sdpRegister(const QBluetoothAddress &local,
                                            QBluetooth::SDPProfile profile,
                                            int channel)
{
    return m_private->sdpRegister(local, profile, channel);
}

/*!
    Unregisters the SDP service for this service provider. This will fail if
    sdpRegister() has not been previously called to register a SDP service.
 */
bool QBluetoothAbstractService::sdpUnregister()
{
    return m_private->sdpUnregister();
}

/*!
    Returns a description of the last error that occured when calling
    sdpRegister() or sdpUnregister() (i.e. if either of them returned \c false).
 */
QString QBluetoothAbstractService::sdpError()
{
    return m_private->sdpError();
}

/*!
    \fn void QBluetoothAbstractService::started(QBluetooth::ServiceError error, const QString &errorDesc)

    This signal should be emitted when the service has started or failed while
    attempting to start. If there was a failure, the error details should be
    provided with the \a error identifier and \a errorDesc description.
    Otherwise, \a error should be QBluetooth::NoError and \a errorDesc should
    be a null QString.
 */

/*!
    \fn void QBluetoothAbstractService::stopped(QBluetooth::ServiceError error, const QString &errorDesc)

    This should must be emitted when the service has stopped or failed while
    attempting to stop. If there was a failure, the error details should be
    provided with the \a error identifier and \a errorDesc description.
    Otherwise, \a error should be QBluetooth::NoError and \a errorDesc should
    be a null QString.
 */

/*!
    \fn void QBluetoothAbstractService::error(QBluetooth::ServiceError error, const QString &errorDesc)

    This should must be emitted when the service has encountered a
    fatal error while running. The error details should be provided with an
    \a error identifier and \a errorDesc description.
 */

#include "qbluetoothabstractservice.moc"
