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

#include <qbluetoothsdpquery.h>
#include <qbluetoothsdpuuid.h>
#include <qbluetoothsdprecord.h>
#include <qbluetoothlocaldevice.h>
#include <qbluetoothaddress.h>
#include <qbluetoothnamespace.h>
#include <qtopialog.h>
#include <qtopianamespace.h>

#include <qtopiacomm/private/qbluetoothnamespace_p.h>
#include <qtopiacomm/private/qsdpxmlparser_p.h>

#include <qtdbus/qdbusconnection.h>
#include <qtdbus/qdbusinterface.h>
#include <qtdbus/qdbusreply.h>
#include <qtdbus/qdbusmessage.h>
#include <qtdbus/qdbusmetatype.h>
#include <qtdbus/qdbusargument.h>

#include <QList>
#include <QString>
#include <QProcess>
#include <QTextStream>
#include <QStack>
#include <QUrl>
#include <QMetaType>

Q_DECLARE_METATYPE(QList<uint>)

/*!
    \class QBluetoothSdpQueryResult
    \brief The QBluetoothSdpQueryResult class encapsulates a result of an SDAP query.

The QBluetoothSdpQueryResult class encapsulates a result of an SDAP query, which is returned by the QBluetoothSdpQuery object.
The query consists of a list of \c QBluetoothSdpRecord objects.

    \ingroup qtopiabluetooth
    \sa QBluetoothSdpQuery, QBluetoothSdpRecord
 */

/*!
    Constructs an empty QBluetoothSdpQueryResult object.
*/
QBluetoothSdpQueryResult::QBluetoothSdpQueryResult()
{
    m_valid = true;
}

/*!
    Construct a QBluetoothSdpQueryResult, copying contents from \a other.
    */
QBluetoothSdpQueryResult::QBluetoothSdpQueryResult(const QBluetoothSdpQueryResult &other)
{
    (*this) = other; //use assignment operator
}

/*!
    Assign the contents of \a other to the current QBluetoothSdpQueryResult.
  */
QBluetoothSdpQueryResult& QBluetoothSdpQueryResult::operator=(const QBluetoothSdpQueryResult &other)
{
    if (this == &other)
        return (*this);

    m_services.clear();
    m_services = other.m_services;
    m_error = other.m_error;
    m_valid = other.m_valid;
    return (*this);
}

/*!
    Adds a \a service to the search result.
*/
void QBluetoothSdpQueryResult::addService(const QBluetoothSdpRecord &service)
{
    m_services.push_back(service);
}

/*!
    Returns whether the search result is valid (no error occurred during the search).
*/
bool QBluetoothSdpQueryResult::isValid() const
{
    return m_valid;
}

/*!
    Returns the error string, if any.
*/
QString QBluetoothSdpQueryResult::error() const
{
    return m_error;
}

/*!
    Flags that an error occurred during an SDAP query and sets the error string to
    \a error.
*/
void QBluetoothSdpQueryResult::setError(const QString &error)
{
    m_error = error;
    m_valid = false;
}

/*!
    Returns a list of services found during an SDAP query.
*/
const QList<QBluetoothSdpRecord> &QBluetoothSdpQueryResult::services() const
{
    return m_services;
}

/*!
    Resets the results object.  This clears the service list, resets the valid flag
    and resets the error message to null.
*/
void QBluetoothSdpQueryResult::reset()
{
    m_services.clear();
    m_valid = true;
    m_error = QString::null;
}

class QBluetoothSdpQuery_Private : public QObject
{
    Q_OBJECT

public:
    QBluetoothSdpQuery_Private(QBluetoothSdpQuery *parent);
    ~QBluetoothSdpQuery_Private();

    void reset();
    bool searchServices(const QBluetoothAddress &remote,
                        const QBluetoothLocalDevice &local,
                        const QString &searchString);
    QBluetoothSdpQuery *m_parent;

    // DBUS Specific stuff
    QDBusInterface *m_iface;
    QList<uint> m_handles;
    int m_index;
    QBluetoothAddress m_remote;

    bool m_finished;
    bool m_cancelled;

    QBluetoothSdpQueryResult m_result;

public slots:
    void asyncGetHandlesReply(const QDBusMessage &msg);
    void asyncGetRecordReply(const QDBusMessage &msg);

private:
    void requestNextRecord();
    bool haveInterface(const QString &device);
};

QBluetoothSdpQuery_Private::QBluetoothSdpQuery_Private(QBluetoothSdpQuery *parent)
    : m_iface(0)
{
    m_parent = parent;
    m_finished = true;
}

QBluetoothSdpQuery_Private::~QBluetoothSdpQuery_Private()
{
}

void QBluetoothSdpQuery_Private::reset()
{
    m_result.reset();
    m_handles.clear();
    m_index = 0;
    m_cancelled = false;
}

void QBluetoothSdpQuery_Private::requestNextRecord()
{
    QList<QVariant> args;
    args << m_remote.toString() << m_handles[m_index];

    bool ret = m_iface->callWithCallback("GetRemoteServiceRecordAsXML", args, this,
            SLOT(asyncGetRecordReply(const QDBusMessage &)));

    if (!ret) {
        m_result.setError("Call failed");
        qLog(Bluetooth) << "Couldn't call GetRemoteServiceRecordAsXML";
        emit m_parent->searchComplete(m_result);
        m_finished = true;
    }
}

void QBluetoothSdpQuery_Private::asyncGetRecordReply(const QDBusMessage &msg)
{
    if (m_cancelled) {
        emit m_parent->searchCancelled();
        m_finished = true;
        return;
    }

    QDBusReply<QString> reply(msg);

    if (!reply.isValid()) {
        m_result.setError(reply.error().message());
        qLog(Bluetooth) << "Received an error reply from hcid:" << reply.error();
        emit m_parent->searchComplete(m_result);
        m_finished = true;
        return;
    }

    // Grab the result here and parse
    QSdpXmlParser parser;
    if (parser.parseRecord(reply.value().toUtf8())) {
        m_result.addService(parser.record());
    } else {
        qLog(Bluetooth) << "Trouble parsing record:" << m_handles[m_index];
        qLog(Bluetooth) << "The error was:" << parser.errorString();
    }

    m_index++;

    if (m_index < m_handles.size()) {
        requestNextRecord();
    } else {
        emit m_parent->searchComplete(m_result);
        m_finished = true;
    }
}

void QBluetoothSdpQuery_Private::asyncGetHandlesReply(const QDBusMessage &msg)
{
    if (m_cancelled) {
        emit m_parent->searchCancelled();
        m_finished = true;
        return;
    }

    QDBusReply<QList<uint> > reply(msg);

    if (!reply.isValid()) {
        m_result.setError(reply.error().message());
        qLog(Bluetooth) << "Received an error during service record request" << reply.error();
        emit m_parent->searchComplete(m_result);
        m_finished = true;
        return;
    }

    m_handles = reply.value();

    // No records found
    if (m_handles.size() == 0) {
        emit m_parent->searchComplete(m_result);
        m_finished = true;
        return;
    }

    requestNextRecord();
}

bool QBluetoothSdpQuery_Private::haveInterface(const QString &device)
{
    if (m_iface && m_iface->isValid()) {
        if (m_iface->path().mid(11) == device) {
            return true;
        }
    }

    if (m_iface) {
        delete m_iface;
        m_iface = 0;
    }

    QDBusConnection dbc = QDBusConnection::systemBus();
    QDBusInterface iface("org.bluez", "/org/bluez",
                         "org.bluez.Manager", dbc);
    if (!iface.isValid()) {
        return false;
    }

    QDBusReply<QString> reply = iface.call("FindAdapter", device);

    if (!reply.isValid()) {
        qLog(Bluetooth) << "No Adapter:" << device << reply.error();
        return false;
    }

    m_iface = new QDBusInterface("org.bluez", reply.value(), "org.bluez.Adapter", dbc);

    if (!m_iface->isValid()) {
        qLog(Bluetooth) << "Could not find org.bluez Adapter interface for" << device;
        delete m_iface;
        m_iface = 0;
        return false;
    }

    return true;
}

bool QBluetoothSdpQuery_Private::searchServices(const QBluetoothAddress &remote,
                                   const QBluetoothLocalDevice &local,
                                   const QString &searchString)
{
    if (!haveInterface(local.deviceName()))
        return false;

    if (!m_finished)
        return false;

    m_finished = false;

    reset();
    m_remote = remote;

    QList<QVariant> args;
    args << remote.toString() << searchString;

    return m_iface->callWithCallback("GetRemoteServiceHandles", args, this,
                                     SLOT(asyncGetHandlesReply(const QDBusMessage &)));
}

/*!
    \class QBluetoothSdpQuery
    \brief The QBluetoothSdpQuery class encapsulates operations performed by the SDAP profile.

    The QBluetoothSdpQuery class provides a way to use the facilities of the Bluetooth Service
    Discovery Application Profile.  QBluetoothSdpQuery allows the client to browse all services
    or search for a specific service on a remote device.  QBluetoothSdpQuery also allows the same
    functionality to be performed for a local device.  The results are reported in the
    \c QBluetoothSdpQueryResult object.

    \ingroup qtopiabluetooth
    \sa QBluetoothSdpQueryResult, QBluetoothSdpRecord
 */

/*!
    Constructs a new QBluetoothSdpQuery object.  The default local device will be used for all
    SDP queries. \a parent is the parent QObject.
 */
QBluetoothSdpQuery::QBluetoothSdpQuery( QObject* parent )
    :QObject( parent )
{
    QBluetoothLocalDevice device;
    m_data = new QBluetoothSdpQuery_Private(this);
}

/*!
    Deconstructs a QBluetoothSdpQuery object.
 */
QBluetoothSdpQuery::~QBluetoothSdpQuery()
{
    if (m_data)
        delete m_data;
}

/*!
    Cancels the SDAP query.  The searchCancelled() signal will be sent once
    the search has been successfully cancelled.  It is an error to start
    a new search after cancelling but before receiving the cancelled notification.

    \sa searchCancelled()
 */
void QBluetoothSdpQuery::cancelSearch()
{
    m_data->m_cancelled = true;
}

/*!
    Starts a new search query that searches for a specific Bluetooth service profile, e.g.
    OBEX Object PUSH.  The remote device to search is specified by the
    \a remote parameter.  The profile to search for is specified by the \a profile parameter.
    The \a local parameter specifies the local Bluetooth adapter to use for the query.

    The method returns true if the request could be started successfully, and false
    otherwise.  The searchComplete() signal will be sent once the search is finished.

    \sa searchComplete()
 */
bool QBluetoothSdpQuery::searchServices(const QBluetoothAddress &remote,
                           const QBluetoothLocalDevice &local,
                           QBluetooth::SDPProfile profile)
{
    return searchServices(remote, local, QBluetoothSdpUuid::fromProfile(profile));
}

/*!
    Starts a new search query.  The remote device to search is
    specified by the \a remote parameter.  All services with attribute UUIDs that
    match a UUID provided by the \a uuid parameter will be returned.

    The method returns true if the request could be started
    successfully, and false otherwise. The searchComplete() signal will
    be sent once the search is finished.

    \sa searchComplete()
 */
bool QBluetoothSdpQuery::searchServices(const QBluetoothAddress &remote,
                           const QBluetoothLocalDevice &local,
                           const QBluetoothSdpUuid &uuid)
{
    return m_data->searchServices(remote, local, uuid.toUuid128().toString());
}

/*!
    Starts a new browse query of the remote device.  This query
    will return all services which are listed in the public browse
    group of the device, and any subgroups which are children of the
    public browse group.  The remote device to search is specified
    by the \a remote parameter.  The \a local parameter specifies
    the local device to use for the query.

    The method returns true if the request could be started
    successfully, and false otherwise.

    Please note, that if you require all services, which might not be
    part of the public browse group, you can try to perform a search
    using the L2CAP UUID (0x0100).

    \code
        QBluetoothSdpQuery qsdap;
        qsdap.searchServices(QBluetoothAddress(remote), QBluetoothLocalDevice(), 0x0100);
    \endcode

    The searchComplete() signal will be sent once the search is finished.

    \sa searchComplete()
 */
bool QBluetoothSdpQuery::browseServices(const QBluetoothAddress &remote, const QBluetoothLocalDevice &local)
{
    return m_data->searchServices(remote, local, QString(""));
}

/*!
    \fn void QBluetoothSdpQuery::searchComplete(const QBluetoothSdpQueryResult &result)

    This signal is emitted when an SDAP result has completed.  The result of the scan
    is reported by the \a result parameter.

    \sa QBluetoothSdpQueryResult
 */

/*!
    \fn void QBluetoothSdpQuery::searchCancelled()

    This signal is emitted whenever an SDAP search has been successfully cancelled.

    \sa cancelSearch()
*/

#include "qbluetoothsdpquery.moc"

