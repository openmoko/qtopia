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

#include <qbluetoothaddress.h>
#include <QBluetoothSdpRecord>
#include <QBluetoothLocalDevice>
#include <qtopiacomm/private/qsdpxmlgenerator_p.h>

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>

#include <QtopiaIpcAdaptor>
#include <qtopialog.h>

#include <QTimer>
#include <QFile>
#include <QBuffer>

#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>


/*
    \internal
    \class Messenger
    Passes messages (over IPC) between a QBluetoothAbstractService and the
    BluetoothServiceManager (in the QPE server).
 */
class Messenger : public QtopiaIpcAdaptor
{
    friend class QBluetoothAbstractService;
    Q_OBJECT
public:
    Messenger(QBluetoothAbstractService *service);

    QBluetoothAbstractService *m_service;

public slots:
    void startService(const QString &name);
    void stopService(const QString &name);
    void setSecurityOptions(const QString &name, QBluetooth::SecurityOptions options);

private slots:
    void registerSelf();
    void started(bool error, const QString &);
    void stopped();

signals:
    void registerService(const QString &name, const QString &translatableName);
    void serviceStarted(const QString &name, bool error, const QString &errorDesc);
    void serviceStopped(const QString &name);
};

Messenger::Messenger(QBluetoothAbstractService *service)
    : QtopiaIpcAdaptor("QPE/BluetoothServiceProviders", service),
      m_service(service)
{
    publishAll(SignalsAndSlots);

    QObject::connect(service, SIGNAL(started(bool, const QString &)),
                     SLOT(started(bool, const QString &)));
    QObject::connect(service, SIGNAL(stopped()), SLOT(stopped()));

    // register the service at the end of this run loop
    QTimer::singleShot(0, this, SLOT(registerSelf()));
}

void Messenger::startService(const QString &name)
{
    if (name == m_service->name())
        m_service->start();
}

void Messenger::stopService(const QString &name)
{
    if (name == m_service->name())
        m_service->stop();
}

void Messenger::setSecurityOptions(const QString &name, QBluetooth::SecurityOptions options)
{
    if (name == m_service->name())
        m_service->setSecurityOptions(options);
}

void Messenger::registerSelf()
{
    emit registerService(m_service->name(), m_service->displayName());
}

void Messenger::started(bool error, const QString &desc)
{
    emit serviceStarted(m_service->name(), error, desc);
}

void Messenger::stopped()
{
    emit serviceStopped(m_service->name());
}


// ========================================

class QBluetoothAbstractServicePrivate : public QObject
{
    Q_OBJECT

public:
    QBluetoothAbstractServicePrivate(const QString &name,
                                     const QString &displayName,
                                     QBluetoothAbstractService *parent);

    quint32 registerRecord(const QString &record);
    bool unregisterRecord(quint32 handle);

    QString m_name;
    QString m_displayName;
};

QBluetoothAbstractServicePrivate::QBluetoothAbstractServicePrivate(const QString &name, const QString &displayName, QBluetoothAbstractService *parent)
    : QObject(parent),
      m_name(name),
      m_displayName(displayName)
{
}

quint32 QBluetoothAbstractServicePrivate::registerRecord(const QString &record)
{
    QDBusConnection dbc = QDBusConnection::systemBus();
    QDBusInterface iface("org.bluez", "/org/bluez",
                        "org.bluez.Database", dbc);
    if (!iface.isValid())
        return 0;

    QDBusReply<quint32> reply = iface.call("AddServiceRecordFromXML", record);
    if (!reply.isValid()) {
        qLog(Bluetooth) << "BluetoothAbstractService: registerRecord() error:"
                << reply.error().type() << reply.error().message();
        return 0;
    }

    return reply.value();
}

bool QBluetoothAbstractServicePrivate::unregisterRecord(quint32 handle)
{
    QDBusConnection dbc = QDBusConnection::systemBus();
    QDBusInterface iface("org.bluez", "/org/bluez",
                        "org.bluez.Database", dbc);
    if (!iface.isValid())
        return false;

    QDBusReply<void> reply = iface.call("RemoveServiceRecord",
                                        QVariant::fromValue(handle));
    if (!reply.isValid()) {
        qLog(Bluetooth) << "BluetoothAbstractService: unregisterRecord() error:"
                << reply.error().type() << reply.error().message();
        return false;
    }

    return true;
}



// ============================================================

/*!
    \class QBluetoothAbstractService
    \brief  The QBluetoothAbstractService class provides a base class for Bluetooth services within Qtopia.

    This class provides the common functionality for running Bluetooth
    services within the Qtopia Bluetooth framework. Each subclass provides
    the specific implementation for running a particular Bluetooth service.

    If you create a Bluetooth service by subclassing QBluetoothAbstractService,
    the service is automatically registered and accessible as a system
    Bluetooth service. The service can then be accessed externally by
    programmers through the QBluetoothServiceController class. It will also be
    listed in the system Bluetooth settings application, allowing end users to
    modify the service's settings.

    Here is an example of creating a custom Bluetooth service using a class
    named \c MyBluetoothService. The service will advertise the Serial
    Port Profile, and the service will be named "MySerialPortService".

    The interface of the class will look like this:
    \code
    class MyBluetoothService : public QObject
    {
        Q_OBJECT
    public:
        MyBluetoothService(QObject *parent = 0);
        virtual void start();
        virtual void stop();
        virtual void setSecurityOptions(QBluetooth::SecurityOptions options);

    private:
        quint32 m_serviceRecordHandle;
    };
    \endcode


    Now, to the implementation. Firstly, the constructor might look something
    like this:

    \code
    MyBluetoothService::MyBluetoothService(QObject *parent)
        : QBluetoothAbstractService("MySerialPortService", tr("Serial Port Service"), parent)
    {
        m_serviceRecordHandle = 0;
    }
    \endcode

    This creates a service named "MySerialPortService". The name must be
    unique among Qtopia Bluetooth services, as it is used internally to
    uniquely identify the service (for example, for storing service settings).

    The second string argument is an internationalized, human-readable name
    that can be displayed to the end user. This string will be used, for
    example, for listing local Bluetooth services in Qtopia's Bluetooth 
    settings application.

    The \c m_serviceRecordHandle member will store the handle for the 
    service's associated SDP service record, as you will see below in the
    start() and stop() implementations.

    Then, to implement the start() method:

    \code
        void MyBluetoothService::start()
        {
            // Register an SDP service for this Bluetooth service
            quint32 handle = registerRecord("SerialPortService.xml");
            if (handle == 0) {
                // registration failed
                emit started(true, tr("Error registering the SDP service"));
                return;
            }

            // Call some method to start the service
            if (!startMyService()) {
                // The service failed to start, so unregister the SDP service
                // that was previously registered
                unregisterRecord(handle);

                // notify the system that the service was not started
                emit started(true, tr("Unable to start MySerialPortService"));
                return;
            }

            // The service was successfully started
            m_serviceRecordHandle = handle;
            emit started(false, QString());
        }
    \endcode

    The start() method is called by Qtopia when the service should be started.
    Note how the example emits the started() signal when the service fails to
    start, and also when it does start successfully.

    The class also needs to have a stop() method:

    \code
        void MyBluetoothService::stop()
        {
            // call some method to shut down the service
            stopMyService();

            if (!unregisterRecord(m_serviceRecordHandle))
                qDebug() << "Error unregistering the SDP service";

            // notify the system that the service has stopped
            emit stopped();
        }
    \endcode

    Finally, the class will need to implement the setSecurityOptions() method
    to provide the ability to modify the security settings for the service.

    \ingroup qtopiabluetooth
*/


/*!
    Construct a Bluetooth service. The \a name is a unique name that identifies
    the service, and \a displayName is a user-friendly, internationalized name
    for this service that can be displayed to the end user. The \a parent is
    the QObject parent for this service.
 */
QBluetoothAbstractService::QBluetoothAbstractService(const QString &name, const QString &displayName, QObject *parent)
    : QObject(parent),
      m_data(new QBluetoothAbstractServicePrivate(name, displayName, this))
{
    new Messenger(this);
}

/*!
    Destroys a Bluetooth service.
 */
QBluetoothAbstractService::~QBluetoothAbstractService()
{
}

/*!
    Registers the SDP service record \a record for this Bluetooth service
    and returns the service record handle of the newly registered service.
    Returns zero if the registration failed.
 */
quint32 QBluetoothAbstractService::registerRecord(const QBluetoothSdpRecord &record)
{
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    QSdpXmlGenerator::generate(record, &buffer);

    buffer.seek(0);
    quint32 r = m_data->registerRecord(QString::fromUtf8(buffer.readAll()));

    if (r == 0) {
        qLog(Bluetooth) << "QBluetoothAbstractService: registerRecord() failed for "
                << record.serviceName();
    } else {
        qLog(Bluetooth) << "QBluetoothAbstractService: registerRecord() successful for"
                << record.serviceName();
    }

    return r;
}

/*!
    Uses the XML data from the file \a filename to register the SDP service
    record for this Bluetooth service and returns the service record handle of 
    the newly registered service. Returns zero if the registration failed.

    An example of the required XML data structure can be seen by running the 
    \c {sdptool get --xml} command in a terminal shell:
    \code
        $> sdptool add OPUSH
        $> sdptool browse local
        Browsing FF:FF:FF:00:00:00 ...
        Service Name: OBEX Object Push
        Service RecHandle: 0x10000
        Service Class ID List:
        "OBEX Object Push" (0x1105)
        Protocol Descriptor List:
        "L2CAP" (0x0100)
        "RFCOMM" (0x0003)
            Channel: 9
        "OBEX" (0x0008)
        Profile Descriptor List:
        "OBEX Object Push" (0x1105)
            Version: 0x0100

        $> sdptool get --xml 0x10000
    \endcode

    \bold {Note:} If you use \c {sdptool get --xml} to generate the SDP record 
    XML file, you \bold must remove the XML element containing the service 
    record handle. This element is usually near the start of the XML output and 
    will look similar to this:

    \code
        <attribute id="0x0000">
                <uint32 value="..." />
        </attribute>
    \endcode

    If this element is not removed, the registration will fail, as the 
    service record handle will be automatically generated by the SDP server
    and should not be provided by the programmer.

    \warning The given file must be UTF-8 encoded to be parsed correctly.
 */
quint32 QBluetoothAbstractService::registerRecord(const QString &filename)
{
    int fd = ::open(QFile::encodeName(filename), O_RDONLY);
    if (fd == -1) {
        qLog(Bluetooth) << "QBluetoothAbstractService: registerRecord() error:"
                << "cannot open file" << filename;
        return 0;
    }

    quint32 result = 0;

    // from QResource
    struct stat st;
    if (fstat(fd, &st) != -1) {
        uchar *ptr;
        ptr = reinterpret_cast<uchar *>(
                mmap(0, st.st_size,             // any address, whole file
                     PROT_READ,                 // read-only memory
                     MAP_FILE | MAP_PRIVATE,    // swap-backed map from file
                     fd, 0));                   // from offset 0 of fd
        if (ptr && ptr != reinterpret_cast<uchar *>(MAP_FAILED)) {
            // register the record
            result = m_data->registerRecord( QString::fromUtf8((const char*)ptr) );

            // unmap to clean up
            munmap(ptr, st.st_size);
        } else {
            qLog(Bluetooth) << "QBluetoothAbstractService: registerRecord() mmap failed";
        }
    } else {
        qLog(Bluetooth) << "QBluetoothAbstractService: registerRecord() fstat failed";
    }
    ::close(fd);

    if (result == 0) {
        qLog(Bluetooth) << "QBluetoothAbstractService: registerRecord() failed for "
                << filename;
    } else {
        qLog(Bluetooth) << "QBluetoothAbstractService: registerRecord() successful for"
                << filename;
    }

    return result;
}

/*!
    Unregisters the SDP service record with the service record handle
    \a handle.

    Returns whether the record was successfully unregistered.
 */
bool QBluetoothAbstractService::unregisterRecord(quint32 handle)
{
    return m_data->unregisterRecord(handle);
}

/*!
    Returns the unique name that identifies this service.
 */
QString QBluetoothAbstractService::name() const
{
    return m_data->m_name;
}


/*!
    Returns the user-friendly, internationalized name for this service that can
    be displayed to the end user.
 */
QString QBluetoothAbstractService::displayName() const
{
    return m_data->m_displayName;
}



/*!
    \fn void QBluetoothAbstractService::start()

    Starts this service.

    This method will be called by Qtopia when the service should be started.
    This may be because an external party has required that the service be
    started (for example, through QBluetoothServiceController, or through the
    Bluetooth Settings application) or because Qtopia has been configured to
    start the service automatically.

    Subclasses must override this to start the service appropriately. A
    subclass must emit started() when the service has started (i.e. at the
    end of this method), or failed while trying to start.

    \warning This function must be implementated in such a way that any intermediate
    objects (which have been created up to the point where the error occured)
    are cleaned up before the error signal is emitted.

    \sa started()
 */

/*!
    \fn void QBluetoothAbstractService::stop()

    Stops this service.

    This method will be called by Qtopia when the service should be stopped.

    Subclasses must override this to stop the service appropriately. A
    subclass must emit stopped() when the service has stopped.

    \sa stopped()
 */

/*!
    \fn void QBluetoothAbstractService::setSecurityOptions(QBluetooth::SecurityOptions options)

    Sets the security options for this service to the given \a options.

    This method will be called by Qtopia when the security options should be
    changed.

    Subclasses must override this to set the security options for this service.
 */

/*!
    \fn void QBluetoothAbstractService::started(bool error, const QString &description)

    This signal should be emitted when the service has started or failed while
    attempting to start.

    If there was a failure, \a error should be true and \a description should
    be a human-readable description of the error. Otherwise, \a error should be
    false and \a description should be a null QString.
 */

/*!
    \fn void QBluetoothAbstractService::stopped()

    This signal should be emitted when the service has stopped.
 */

#include "qbluetoothabstractservice.moc"
