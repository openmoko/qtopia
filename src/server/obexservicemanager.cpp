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

#include "obexservicemanager.h"
#include <qobexpushservice.h>
#include <qobexpushclient.h>
#include <qcommdevicesession.h>

#ifdef QTOPIA_BLUETOOTH
#include <qsdp.h>
#include <qsdpservice.h>
#include <qbluetoothabstractservice.h>
#include <qbluetoothobexserver.h>
#include <qbluetoothlocaldevice.h>
#include <qbluetoothlocaldevicemanager.h>
#include <qbluetoothnamespace.h>
#include <qbluetoothobexsocket.h>
#include <qsdap.h>
#include <qbluetoothdeviceselector.h>
#endif

#ifdef QTOPIA_INFRARED
#include <qirlocaldevice.h>
#include <qirobexserver.h>
#include <qirnamespace.h>
#include <qirobexsocket.h>
#endif

#include <qvaluespace.h>
#include <qwaitwidget.h>
#include <qcontent.h>
#include <qmimetype.h>
#include <qtopiaabstractservice.h>
#include <qdsactionrequest.h>
#include <qdsdata.h>
#include <qtopialog.h>
#include <qtopianamespace.h>
#include <qobexsocket.h>
#include <qtopia/pim/qcontactmodel.h>
#include <qcommdevicesession.h>

#include <QProcess>
#include <QStringList>
#include <QTimer>
#include <QDebug>

#include <unistd.h>
#include <sys/vfs.h>

#define OBEX_INCOMING_DIRECTORY (Qtopia::tempDir()+"obex/in/")

class CustomPushService : public QObexPushService
{
    Q_OBJECT
public:
    CustomPushService(QObexSocket *socket, QObject *parent = 0);

    QByteArray businessCard() const;

protected:
    virtual bool acceptFile(const QString &filename, const QString &mimetype, qint64 size);
};

CustomPushService::CustomPushService(QObexSocket *socket, QObject *parent)
    : QObexPushService(socket, parent)
{
}

static QString pretty_print_size(qint64 fsize)
{
    static int threshold = 1024*1024; // 1 MB, user's don't care about fractions up to this point
    static const char *size_suffix[] = {
        "",
        QT_TRANSLATE_NOOP("CustomPushService", "KB"),
        QT_TRANSLATE_NOOP("CustomPushService", "MB"),
        QT_TRANSLATE_NOOP("CustomPushService", "GB"),
    };

    double max = fsize;

    int i = 0;
    for (; i < 4; i++) {
        if (max > 1024.0) {
            max /= 1024.0;
        }
        else {
            break;
        }
    }

    // REALLY big file?
    if (i == 4)
        i = 0;

    QString size;
    if (fsize >= threshold) {
        size = QString::number(max, 'f', 1);
    } else {
        size = QString::number( static_cast<qint64>(max) );
    }
    size += qApp->translate( "CustomPushService", size_suffix[i] );

    return size;
}

bool CustomPushService::acceptFile(const QString &filename,
                                   const QString &mimetype,
                                   qint64 size)
{
    qLog(Obex) << "CustomPushService::acceptFile";

    // This checks that the incoming directory is set
    if (QObexPushService::acceptFile(filename, mimetype, size)) {

        qint64 availableStorage;

        struct statfs fs;
        if ( statfs( incomingDirectory().toLocal8Bit(), &fs ) == 0 ) {
            availableStorage = fs.f_bavail * fs.f_bsize;
        }
        else {
            qWarning("Could not stat: %s",
                    incomingDirectory().toLocal8Bit().constData());
            return false;
        }

        // Accept small files of 10K or less and mimetype vCard or vCalendar
        if ( (size < 1024*10) && (size < availableStorage) ) {
            QString mime = mimetype.toLower();
            if (mime == "text/x-vcard")
                return true;
            if (mime == "text/x-vcalendar")
                return true;
        }

        int result = QMessageBox::question(0, tr("Accept file?"),
                                            tr("<qt>Would you like to accept file:"
                                                " <b>%1</b> of size:"
                                                " %2?  You have %3 of storage"
                                                " remaining.</qt>"
                                                ).arg(filename).
                                                arg(pretty_print_size(size)).
                                                    arg(pretty_print_size(availableStorage)),
                                            QMessageBox::Yes|QMessageBox::Default,
                                            QMessageBox::No|QMessageBox::Escape);
        if (result == QMessageBox::Yes) {
            return true;
        }

        return false;
    }

    return false;
}

QByteArray CustomPushService::businessCard() const
{
    QContactModel model(0);

    if (!model.hasPersonalDetails()) {
        return QByteArray();
    }

    QContact myVCard = model.personalDetails();
    QByteArray arr;
    QDataStream stream(&arr, QIODevice::WriteOnly);
    myVCard.writeVCard(&stream);
    return arr;
}

#ifdef QTOPIA_INFRARED
class InfraredBeamingService : public QtopiaAbstractService
{
    Q_OBJECT

public:
    InfraredBeamingService(ObexServiceManager *parent);
    ~InfraredBeamingService();

public slots:
    void beamPersonalBusinessCard();
    void beamBusinessCard(const QContact &contact);
    void beamBusinessCard(const QDSActionRequest &request);

    void beamFile(const QString &filename, const QString &mimetype, bool autodelete);
    void beamFile(const QContentId &id);

    void beamCalendar(const QDSActionRequest &request);

private slots:
    void doneBeamingVObj(bool error);
    void doneBeamingFile(bool error);

    void sessionOpen();
    void sessionFailed();

private:
    void startBeamingFile(const QString &filename, const QString &mimetype, bool autodelete);
    void startBeamingVObj(const QByteArray &data, const QString &mimetype);

    bool m_busy;
    ObexServiceManager *m_parent;
    QWaitWidget *m_waitWidget;
    QFile *m_file;
    QDSActionRequest *m_current;
    bool m_autodelete;
    QString m_filename;
    QString m_mimetype;
    QCommDeviceSession *m_session;
    QByteArray m_deviceName;

    enum Type {VObject, File};
    Type m_type;
    QByteArray m_vobj;
};

/*!
    \service InfraredBeamingService InfraredBeaming
    \brief Provides the Qtopia InfraredBeaming service.

    The \i InfraredBeaming service enables applications to send
    files over the Infrared link.  The service takes care of
    establishing a connection, tracking progress and notifying the user
    of events that occur during the transmission.
 */

/*!
    \internal
*/
InfraredBeamingService::InfraredBeamingService(ObexServiceManager *parent)
    : QtopiaAbstractService("InfraredBeaming", parent)
        , m_waitWidget(0)
        , m_file(0)
        , m_session(0)
{
    QStringList irDevices = QIrLocalDevice::devices();
    m_deviceName = irDevices[0].toLatin1();

    m_parent = parent;
    m_busy = false;
    m_waitWidget = new QWaitWidget(0);
    m_file = 0;
    m_current = 0;

    m_session = 0;

    publishAll();
}

/*!
    \internal
 */
InfraredBeamingService::~InfraredBeamingService()
{
    delete m_waitWidget;
    delete m_session;
    delete m_file;
}

/*!
    \internal
 */
void InfraredBeamingService::sessionFailed()
{
    if (m_type == VObject) {
        m_waitWidget->setText(tr( "<P>There was an error using the local infrared device."));
        m_waitWidget->setCancelEnabled(true);
        QTimer::singleShot(5000, m_waitWidget, SLOT(hide()));
    }
    else {
        QMessageBox::critical( 0, tr( "Beam File" ),
                              tr( "<P>Error while trying to use the infrared device." ));
    }

    m_busy = false;
}

/*!
    \internal
 */
void InfraredBeamingService::sessionOpen()
{
    QIrObexSocket *socket = new QIrObexSocket("OBEX");
    bool conn = socket->connect();
    if (!conn) {
        m_session->endSession();
        if (m_type == VObject) {
            m_waitWidget->setText(tr( "<P>Could not connect to the remote device.  Please make sure that the Infrared ports are aligned and the device's Infrared capabilities are turned on."));
            m_waitWidget->setCancelEnabled(true);
            QTimer::singleShot(5000, m_waitWidget, SLOT(hide()));
        }
        else {
            QMessageBox::warning( 0, tr( "Beam File" ),
                              tr( "<P>Could not connect to the remote device.  Please make sure that the Infrared ports are aligned and the device's Infrared capabilities are turned on." ));
        }

        m_busy = false;
        return;
    }

    QObexPushClient *client = new QObexPushClient(socket, this);
    client->setAutoDelete(true);

    if (m_type == File) {
        QObject::connect(client, SIGNAL(done(bool)), this, SLOT(doneBeamingFile(bool)));
        m_parent->setupConnection(client, m_filename, m_mimetype);
    }
    else {
        QObject::connect(client, SIGNAL(done(bool)), this, SLOT(doneBeamingVObj(bool)));
        m_waitWidget->setText("Sending...");
    }

    client->connect();
    if (m_file) {
        client->send(m_file, m_filename, m_mimetype);
    }
    else {
        client->send(m_vobj, m_filename, m_mimetype);
    }
    client->disconnect();
}

/*!
    \internal
 */
void InfraredBeamingService::startBeamingVObj(const QByteArray &data, const QString &mimetype)
{
    m_waitWidget->setText("Connecting...");
    m_waitWidget->setCancelEnabled(false);
    m_waitWidget->show();

    m_mimetype = mimetype;
    if (m_mimetype == "text/x-vcard") {
        m_filename = "MyBusinessCard.vcf";
    }
    else if (m_mimetype == "text/x-vcalendar") {
        m_filename = "vcal.vcs";
    }
    m_vobj = data;
    m_type = VObject;

    m_busy = true;

    if (!m_session) {
        qLog(Infrared) << "Lazy initializing a QCommDeviceSession object";
        m_session = new QCommDeviceSession(m_deviceName, this);
        QObject::connect(m_session, SIGNAL(sessionOpen()), this, SLOT(sessionOpen()));
        QObject::connect(m_session, SIGNAL(sessionFailed()), this, SLOT(sessionFailed()));
    }

    m_session->startSession();
}

/*!
    \internal
 */
void InfraredBeamingService::doneBeamingVObj(bool error)
{
    m_busy = false;

    if (error) {
        if (m_current) {
            m_current->respond(tr("Transmission error"));
            delete m_current;
            m_current = 0;
        }

        m_waitWidget->setText(tr( "<P>An error has occurred during transmission.  Please make sure that the Infrared ports are kept aligned and the device's Infrared capabilities are turned on.") );
        m_waitWidget->setCancelEnabled(true);
        QTimer::singleShot(5000, m_waitWidget, SLOT(hide()));
        return;
    }

    if (m_current) {
        m_current->respond();
        delete m_current;
        m_current = 0;
    }

    m_session->endSession();
    m_waitWidget->setText("Sending...Done");
    QTimer::singleShot(1000, m_waitWidget, SLOT(hide()));
}

/*!
    \internal
 */
void InfraredBeamingService::startBeamingFile(const QString &filename,
                                              const QString &mimetype,
                                              bool autodelete)
{
    m_file = new QFile(filename);

    if (!m_file->open(QIODevice::ReadOnly)) {
        delete m_file;
        m_file = 0;
        QMessageBox::warning( 0, tr( "Beam File" ),
                              tr( "<P>The file you are trying to beam could not be opened!" ));
        return;
    }

    m_type = File;
    m_filename = filename;
    m_mimetype = mimetype;
    m_autodelete = autodelete;

    m_busy = true;

    if (!m_session) {
        qLog(Infrared) << "Lazy initializing a QCommDeviceSession object";
        m_session = new QCommDeviceSession(m_deviceName, this);
        QObject::connect(m_session, SIGNAL(sessionOpen()), this, SLOT(sessionOpen()));
        QObject::connect(m_session, SIGNAL(sessionFailed()), this, SLOT(sessionFailed()));
    }

    m_session->startSession();
}

/*!
    \internal
 */
void InfraredBeamingService::doneBeamingFile(bool error)
{
    Q_UNUSED(error);

    m_session->endSession();
    m_busy = false;
    delete m_file;
    m_file = 0;

    m_session->endSession();

    if (m_autodelete)
        ::unlink(m_filename.toLocal8Bit());
}

/*!
    Asks the service to beam the currently set personal business card to a remote
    device.  If no business card is set, the user will be notified appropriately.
*/
void InfraredBeamingService::beamPersonalBusinessCard()
{
    if (m_busy)
        return;

    QContactModel model(this);

    if (!model.hasPersonalDetails()) {
        QMessageBox::warning( 0, tr( "Beam Personal vCard" ),
                                 tr( "<P>There is no personal business card set.  Please set a personal business card in the Contacts application." ));
        return;
    }

    QContact myVCard = model.personalDetails();
    QByteArray arr;
    QDataStream stream(&arr, QIODevice::WriteOnly);
    myVCard.writeVCard(&stream);

    startBeamingVObj(arr, "text/x-vcard");
}

/*!
    Asks the service to beam the business card represented by \a contact.
*/
void InfraredBeamingService::beamBusinessCard(const QContact &contact)
{
    if (m_busy)
        return;

    QByteArray arr;
    QDataStream stream(&arr, QIODevice::WriteOnly);
    contact.writeVCard(&stream);

    startBeamingVObj(arr, "text/x-vcard");
}

/*!
    Asks the service to beam the business card represented by \a request.  The
    request object should contain raw serialized QContact data.
*/
void InfraredBeamingService::beamBusinessCard(const QDSActionRequest &request)
{
    if (m_busy) {
        QDSActionRequest(request).respond(tr("A transfer operation is in progress."));
        return;
    }

    QByteArray arr = request.requestData().data();
    m_current = new QDSActionRequest(request);
    startBeamingVObj(arr, "text/x-vcard");
}

/*!
    Asks the service to beam a vCalendar object represented by \a request.  The
    request object should contain raw serialized QTask or QAppointment data.
 */
void InfraredBeamingService::beamCalendar(const QDSActionRequest &request)
{
    if (m_busy) {
        QDSActionRequest(request).respond(tr("A transfer operation is in progress."));
        return;
    }

    QByteArray arr = request.requestData().data();
    m_current = new QDSActionRequest(request);
    startBeamingVObj(arr, "text/x-vcalendar");
}

/*!
    Asks the service to beam \a filename to the remote device.  Mimetype
    of the file is given by \a mimetype.  The \a autodelete parameter
    specifies whether the service should delete the file after it has performed
    the operation.
*/
void InfraredBeamingService::beamFile(const QString &filename,
                                      const QString &mimetype,
                                      bool autodelete)
{
    if (m_busy)
        return;

    startBeamingFile(filename, mimetype, autodelete);
}

/*!
    Asks the service to beam a filename represented by \a id.
    A QContent object is built using the \a id.  The filename and
    mimetype are then inferred from the QContent object.
 */
void InfraredBeamingService::beamFile(const QContentId &id)
{
    if (m_busy)
        return;

    QContent content(id);
    QMimeType mime(content);

    startBeamingFile(content.file(), mime.id(), false);
}
#endif

#ifdef QTOPIA_BLUETOOTH
struct BluetoothPushRequest
{
    enum Type { File, Data };

    Type m_type;
    QByteArray m_vobj;
    QBluetoothAddress m_addr;
    QString m_mimetype;
    QString m_filename;
    bool m_autodelete;
};

class BluetoothPushingService : public QtopiaAbstractService
{
    Q_OBJECT

public:
    BluetoothPushingService(ObexServiceManager *parent);
    ~BluetoothPushingService();

public slots:
    void pushPersonalBusinessCard();
    void pushPersonalBusinessCard(const QBluetoothAddress &addr);

    void pushBusinessCard(const QContact &contact);
    void pushBusinessCard(const QDSActionRequest& request);

    void pushCalendar(const QDSActionRequest &request);

    void pushFile(const QString &filename, const QString &mimetype, bool autodelete);
    void pushFile(const QContentId &id);
    void pushFile(const QBluetoothAddress &addr, const QContentId &id);

private slots:
    void sdapQueryComplete(const QSDAPSearchResult &result);
    void donePushingVObj(bool error);
    void donePushingFile(bool error);

    void sessionOpen();
    void sessionFailed();

private:
    void startPushingVObj(const QBluetoothAddress &addr,
                          const QString &mimetype);
    bool getPersonalVCard(QByteArray &arr);

    void startPushingFile();

    void startSession();

    ObexServiceManager *m_parent;
    bool m_busy;
    QWaitWidget *m_waitWidget;
    QSDAP *m_sdap;

    QDSActionRequest *m_current;
    QIODevice *m_device;
    BluetoothPushRequest m_req;
    QCommDeviceSession *m_session;
};

/*!
    \service BluetoothPushingService BluetoothPush
    \brief Provides the Qtopia BluetoothPush service.

    The \i BluetoothPush service enables applications to send
    files over the Bluetooth link.  This service takes care of user interaction
    to select a device to send to (if required), looking up the SDP record
    of the device to find out the RFCOMM channel to connect to, establishing
    the connection to the remote service, tracking progress and notifying the user
    of events that occur during the transmission.
 */

/*!
    \internal
*/
BluetoothPushingService::BluetoothPushingService(ObexServiceManager *parent)
    : QtopiaAbstractService("BluetoothPush")
{
    m_parent = parent;
    m_busy = false;
    m_waitWidget = new QWaitWidget(0);

    m_sdap = new QSDAP();
    QObject::connect(m_sdap, SIGNAL(searchComplete(const QSDAPSearchResult)),
                     this, SLOT(sdapQueryComplete(const QSDAPSearchResult &)));

    m_device = 0;
    m_current = 0;

    m_session = 0;

    publishAll();
}

/*!
    \internal
*/
BluetoothPushingService::~BluetoothPushingService()
{
    delete m_waitWidget;
    delete m_sdap;
}

/*!
    \internal
 */
void BluetoothPushingService::sessionOpen()
{
    if (!m_req.m_addr.valid()) {
        QSet<QBluetooth::SDPProfile> profiles;
        profiles.insert(QBluetooth::ObjectPushProfile);
        m_req.m_addr =
                QBluetoothDeviceSelector::getRemoteDevice(profiles);

        if ( !m_req.m_addr.valid() ) {
            m_session->endSession();
            m_busy = false;
            return;
        }
    }

    if (m_req.m_type == BluetoothPushRequest::Data) {
        startPushingVObj(m_req.m_addr, m_req.m_mimetype);
    }
    else {
        startPushingFile();
    }
}

/*!
    \internal
 */
void BluetoothPushingService::sessionFailed()
{
    m_busy = false;
    QMessageBox::warning( 0, tr( "Bluetooth" ),
                          tr( "<P>There is a problem with the Bluetooth device!" ));
}

/*!
    \internal
 */
void BluetoothPushingService::startSession()
{
    if (!m_session) {
        qLog(Bluetooth) << "Lazy initializing a new QCommDeviceSession object";
        QBluetoothLocalDevice local;
        m_session = new QCommDeviceSession(local.deviceName().toLatin1(), this);
        QObject::connect(m_session, SIGNAL(sessionOpen()), this, SLOT(sessionOpen()));
        QObject::connect(m_session, SIGNAL(sessionFailed()), this, SLOT(sessionFailed()));
    }

    m_session->startSession();
}

/*!
    \internal
 */
void BluetoothPushingService::sdapQueryComplete(const QSDAPSearchResult &result)
{
    qLog(Bluetooth) << "Service searching complete";

    bool success = false;
    int channel;

    foreach ( QSDPService service, result.services() ) {
        if ( QSDPService::isInstance( service, QBluetooth::ObjectPushProfile ) ) {
            channel = QSDPService::rfcommChannel(service);
            success = true;
            break;
        }
    }

    if (!success) {
        m_busy = false;
        m_session->endSession();
        // This should not happen if the DeviceSelector is working correctly
        m_waitWidget->setText("<P>The selected device does not support Object Push Profile.");
        m_waitWidget->setCancelEnabled(true);
        QTimer::singleShot(5000, m_waitWidget, SLOT(hide()));
        return;
    }

    QBluetoothObexSocket *socket =
            new QBluetoothObexSocket( m_req.m_addr, channel, QBluetoothAddress::any);

    bool conn = socket->connect();

    if (!conn) {
        m_busy = false;
        qLog(Bluetooth) << "BluetoothPushingService: ending session";
        m_session->endSession();

        m_waitWidget->setText("<P>Could not connect to the selected bluetooth device.  Please make sure that the device is turned on and within range.");
        m_waitWidget->setCancelEnabled(true);
        QTimer::singleShot(5000, m_waitWidget, SLOT(hide()));
        return;
    }

    QObexPushClient *client = new QObexPushClient(socket, this);
    client->setAutoDelete(true);

    if ((m_req.m_mimetype == "text/x-vcard") || (m_req.m_mimetype == "text/x-vcalendar")) {
        QObject::connect(client, SIGNAL(done(bool)), this, SLOT(donePushingVObj(bool)));
        m_waitWidget->setText("Sending...");
    }
    else {
        m_waitWidget->hide();
        QObject::connect(client, SIGNAL(done(bool)), this, SLOT(donePushingFile(bool)));
        m_parent->setupConnection(client, m_req.m_filename, m_req.m_mimetype);
    }

    client->connect();
    if (m_device) {
        client->send(m_device, m_req.m_filename, m_req.m_mimetype);
    }
    else {
        client->send(m_req.m_vobj, m_req.m_filename, m_req.m_mimetype);
    }
    client->disconnect();
}

/*!
    \internal
 */
void BluetoothPushingService::startPushingVObj(const QBluetoothAddress &addr,
                                               const QString &mimetype)
{
    if (mimetype == "text/x-vcard") {
        m_req.m_filename = "MyBusinessCard.vcf";
    }
    else {
        m_req.m_filename = "vcal.vcs";
    }

    QBluetoothLocalDevice localDev;
    m_sdap->searchServices(addr, localDev, QBluetooth::ObjectPushProfile);

    m_waitWidget->setText("Connecting...");
    m_waitWidget->setCancelEnabled(false);
    m_waitWidget->show();
}

/*!
    \internal
 */
void BluetoothPushingService::donePushingVObj(bool error)
{
    m_busy = false;

    if (error) {
        // Respond to the request
        if (m_current) {
            m_current->respond(tr("Error during transmission"));
            delete m_current;
            m_current = 0;
        }

        m_waitWidget->setText(tr( "<P>An error has occurred during transmission.  Please ensure that the selected Bluetooth device is kept on and in range.") );
        QTimer::singleShot(5000, m_waitWidget, SLOT(hide()));
        m_waitWidget->setCancelEnabled(true);
        return;
    }

    m_waitWidget->setText("Sending...Done");
    QTimer::singleShot(1000, m_waitWidget, SLOT(hide()));

    // Respond to the request
    if (m_current) {
        m_current->respond();
        delete m_current;
        m_current = 0;
    }

    m_session->endSession();
    delete m_session;
    m_session = 0;
}

/*!
    \internal
 */
void BluetoothPushingService::startPushingFile()
{
    m_device = new QFile(m_req.m_filename);

    if (!m_device->open(QIODevice::ReadOnly)) {
        delete m_device;
        m_device = 0;
        QMessageBox::warning( 0, tr( "Push File" ),
                              tr( "<P>The file you are trying to beam could not be opened!" ));
        m_busy = false;
        return;
    }

    QBluetoothLocalDevice localDev;
    m_sdap->searchServices(m_req.m_addr, localDev, QBluetooth::ObjectPushProfile);

    m_waitWidget->setText("Connecting...");
    m_waitWidget->setCancelEnabled(false);
    m_waitWidget->show();
}

/*!
    \internal
 */
void BluetoothPushingService::donePushingFile(bool error)
{
    Q_UNUSED(error);

    m_busy = false;
    delete m_device;
    m_device = 0;

    if (m_req.m_autodelete)
        ::unlink(m_req.m_filename.toLocal8Bit());

    m_session->endSession();
    delete m_session;
    m_session = 0;
}

/*!
    \internal
 */
bool BluetoothPushingService::getPersonalVCard(QByteArray &arr)
{
    QContactModel model(this);

    if (!model.hasPersonalDetails()) {
        QMessageBox::warning( 0, tr( "Push Personal vCard" ),
                              tr( "<P>There is no personal business card set.  Please set a personal business card in the Contacts application." ));
        return false;
    }

    QContact myVCard = model.personalDetails();
    QDataStream stream(&arr, QIODevice::WriteOnly);
    myVCard.writeVCard(&stream);

    return true;
}

/*!
    Asks the service to send the currently set personal business card to a remote
    device.  If no business card is set, the user will be notified appropriately.
 */
void BluetoothPushingService::pushPersonalBusinessCard()
{
    qLog(Bluetooth) << "BluetoothPushingService::pushPersonalBusinessCard()" << m_busy;

    if (m_busy)
        return;

    QByteArray arr;
    if (!getPersonalVCard(arr))
        return;

    m_req.m_type = BluetoothPushRequest::Data;
    m_req.m_vobj = arr;
    m_req.m_addr = QBluetoothAddress();
    m_req.m_mimetype = "text/x-vcard";
    m_req.m_autodelete = false;

    m_busy = true;
    startSession();
}

/*!
    Asks the service to beam the currently set personal business card to a remote
    device.  If no business card is set, the user will be notified appropriately.

    The user will not be prompted to select a remote device to send to, instead
    the a device with address given by \a addr will be used.
 */
void BluetoothPushingService::pushPersonalBusinessCard(const QBluetoothAddress &addr)
{
    qLog(Bluetooth) << "BluetoothPushingService::pushPersonalBusinessCard(addr)" << m_busy;

    if (m_busy)
        return;

    QByteArray arr;
    if (!getPersonalVCard(arr))
        return;

    if ( !addr.valid() ) {
        return;
    }

    m_req.m_type = BluetoothPushRequest::Data;
    m_req.m_vobj = arr;
    m_req.m_addr = addr;
    m_req.m_mimetype = "text/x-vcard";
    m_req.m_autodelete = false;

    m_busy = true;
    startSession();
}

/*!
    Asks the service to send the business card represented by \a contact.
 */
void BluetoothPushingService::pushBusinessCard(const QContact &contact)
{
    qLog(Bluetooth) << "BluetoothPushingService::pushBusinessCard" << m_busy;

    if (m_busy)
        return;

    QByteArray arr;
    QDataStream stream(&arr, QIODevice::WriteOnly);
    contact.writeVCard(&stream);

    m_req.m_type = BluetoothPushRequest::Data;
    m_req.m_vobj = arr;
    m_req.m_addr = QBluetoothAddress();
    m_req.m_mimetype = "text/x-vcard";
    m_req.m_autodelete = false;

    m_busy = true;
    startSession();
}

/*!
    Asks the service to send the business card represented by \a request.  The
    request object should contain raw serialized QContact data.
 */
void BluetoothPushingService::pushBusinessCard(const QDSActionRequest &request)
{
    qLog(Bluetooth) << "BluetoothPushingService::pushBusinessCard" << m_busy;

    if (m_busy)
        return;

    QByteArray arr = request.requestData().data();

    m_req.m_type = BluetoothPushRequest::Data;
    m_req.m_vobj = arr;
    m_req.m_addr = QBluetoothAddress();
    m_req.m_mimetype = "text/x-vcard";
    m_req.m_autodelete = false;

    m_current = new QDSActionRequest(request);

    m_busy = true;
    startSession();
}

/*!
    Asks the service to beam a vCalendar object represented by \a request.  The
    request object should contain raw serialized QTask or QAppointment data.
 */
void BluetoothPushingService::pushCalendar(const QDSActionRequest &request)
{
    if (m_busy)
        return;

    QByteArray arr = request.requestData().data();

    m_req.m_type = BluetoothPushRequest::Data;
    m_req.m_vobj = arr;
    m_req.m_addr = QBluetoothAddress();
    m_req.m_mimetype = "text/x-vcalendar";
    m_req.m_autodelete = false;

    m_current = new QDSActionRequest(request);

    m_busy = true;
    startSession();
}

/*!
    Asks the service to send \a filename to the remote device.  Mimetype
    of the file is given by \a mimetype.  The \a autodelete parameter
    specifies whether the service should delete the file after it has performed
    the operation.
 */
void BluetoothPushingService::pushFile(const QString &filename,
                                       const QString &mimetype,
                                       bool autodelete)
{
    if (m_busy)
        return;

    m_req.m_type = BluetoothPushRequest::File;
    m_req.m_addr = QBluetoothAddress();
    m_req.m_filename = filename;
    m_req.m_mimetype = mimetype;
    m_req.m_autodelete = autodelete;

    m_busy = true;
    startSession();
}

/*!
    Asks the service to beam a filename represented by \a id.
    A QContent object is built using the \a id.  The filename and
    mimetype are then inferred from the QContent object.
 */
void BluetoothPushingService::pushFile(const QContentId &id)
{
    if (m_busy)
        return;

    QContent content(id);
    QMimeType mime(content);

    m_req.m_type = BluetoothPushRequest::File;
    m_req.m_addr = QBluetoothAddress();
    m_req.m_filename = content.file();
    m_req.m_mimetype = mime.id();
    m_req.m_autodelete = false;

    m_busy = true;
    startSession();
}

/*!
    Asks the service to beam a filename represented by \a id.
    A QContent object is built using the \a id.  The filename and
    mimetype are then inferred from the QContent object.

    The user will not be prompted to select a device to send to, instead the device
    at address \a addr will be used.
 */
void BluetoothPushingService::pushFile(const QBluetoothAddress &addr, const QContentId &id)
{
    if (m_busy)
        return;

    QContent content(id);
    QMimeType mime(content);

    m_req.m_type = BluetoothPushRequest::File;
    m_req.m_addr = addr;
    m_req.m_filename = content.file();
    m_req.m_mimetype = mime.id();
    m_req.m_autodelete = false;

    m_busy = true;
    startSession();
}
#endif

#ifdef QTOPIA_BLUETOOTH
class ObexPushServiceProvider : public QBluetoothAbstractService
{
    Q_OBJECT
public:
    ObexPushServiceProvider(ObexServiceManager *parent = 0);
    ~ObexPushServiceProvider();

    virtual void start(int channel);
    virtual void stop();

    virtual void setSecurityOptions(QBluetooth::SecurityOptions options);
    virtual QString translatableDisplayName() const;

private slots:
    void newOPushConnection();
    void sessionEnded();

private:
    void close();

    ObexServiceManager *m_parent;
    QBluetoothObexServer *m_opush;
    QBluetooth::SecurityOptions m_securityOptions;
    QBluetoothLocalDevice *m_local;
    int m_numBtSessions;
    QCommDeviceSession *m_btDeviceSession;
};

ObexPushServiceProvider::ObexPushServiceProvider(ObexServiceManager *parent)
    : QBluetoothAbstractService("ObexObjectPush", parent),
      m_parent(parent),
      m_opush(0),
      m_securityOptions(0),
      m_local(new QBluetoothLocalDevice(this)),
      m_numBtSessions(0),
      m_btDeviceSession(0)
{
    initialize();
}

ObexPushServiceProvider::~ObexPushServiceProvider()
{
    delete m_btDeviceSession;
}

void ObexPushServiceProvider::close()
{
    qLog(Bluetooth) << "ObexPushServiceProvider close";

    if (m_opush) {
        m_opush->close();
        delete m_opush;
        m_opush = 0;
    }

    delete m_btDeviceSession;
    m_btDeviceSession = 0;
}

void ObexPushServiceProvider::start(int channel)
{
    qLog(Bluetooth) << "ObexPushServiceProvider start";
    if (m_opush)
        close();

    if (!m_local->isValid()) {
        delete m_local;
        m_local = new QBluetoothLocalDevice(this);
        if (!m_local->isValid()) {
            emit started(QBluetooth::NoSuchAdapter,
                    tr("Cannot access local bluetooth device"));
            return;
        }
    }

    m_opush = new QBluetoothObexServer(channel, m_local->address(), this);
    connect(m_opush, SIGNAL(newConnection()),
            this, SLOT(newOPushConnection()));

    if (m_opush->isListening()) {
        emit started(QBluetooth::AlreadyRunning,
                   tr("OBEX Push Server already listening"));
        return;
    }

    if (!m_opush->listen()) {
        close();
        emit started(QBluetooth::UnknownError,
                   tr("Error listening on OBEX Push Server"));
        return;
    }

    m_opush->setSecurityOptions(m_securityOptions);

    if (!sdpRegister(m_local->address(), QBluetooth::ObjectPushProfile, channel)) {
        close();
        emit started(QBluetooth::SDPServerError,
                     tr("Error registering with SDP server"));
        return;
    }

    emit started(QBluetooth::NoError, QString());

    if (!m_btDeviceSession) {
        QBluetoothLocalDevice dev;
        m_btDeviceSession = new QCommDeviceSession(dev.deviceName().toLatin1());
    }
}

void ObexPushServiceProvider::newOPushConnection()
{
    if (!m_opush->hasPendingConnections())
        return;

    CustomPushService *opush =
            new CustomPushService(m_opush->nextPendingConnection());
    m_parent->setupConnection(opush);

    connect(opush, SIGNAL(destroyed(QObject *)), this, SLOT(sessionEnded()));

    m_numBtSessions++;

    if (m_numBtSessions == 1) { // First session
        qLog(Bluetooth) << "ObexPushServiceProvider starting BT Session";
        m_btDeviceSession->startSession();
    }
}

void ObexPushServiceProvider::sessionEnded()
{
    m_numBtSessions--;
    qLog(Bluetooth) << "Bluetooth Obex Push session finished, m_numBtSessions: " << m_numBtSessions;

    if (m_numBtSessions == 0) {
        qLog(Bluetooth) << "Ending Bluetooth Obex Push session";
        m_btDeviceSession->endSession();
        qLog(Bluetooth) << "Session ended";
    }
}

void ObexPushServiceProvider::stop()
{
    qLog(Bluetooth) << "ObexPushServiceProvider stop";

    if (!m_opush || !m_opush->isListening()) {
        emit stopped(QBluetooth::NotRunning,
                   tr("OBEX Push Server is not running"));
        return;
    }
    close();

    if (!sdpUnregister()) {
        emit stopped(QBluetooth::SDPServerError,
                     tr("Error unregistering from SDP server"));
        return;
    }

    emit stopped(QBluetooth::NoError, QString());
}

void ObexPushServiceProvider::setSecurityOptions(QBluetooth::SecurityOptions options)
{
    m_securityOptions = options;
    if (m_opush && m_opush->isListening())
        m_opush->setSecurityOptions(options);
}

QString ObexPushServiceProvider::translatableDisplayName() const
{
    return tr("OBEX Object Push Service");
}

#endif


/*!
    \class ObexServiceManager
    \ingroup QtopiaServer
    \brief The ObexServiceManager class is responsible for managing OBEX related services over Bluetooth and Infrared.

    ObexServiceManager provides a common infrastructure for OBEX related services, such as
    the OPUSH Bluetooth profile and IrXfer Infrared profile. 
  */

/*!
    Constructs an ObexServiceManager.  The \a parent argument contains the
    QObject parent.
*/
ObexServiceManager::ObexServiceManager(QObject *parent) : QObject(parent)
#ifdef QTOPIA_INFRARED
        , m_irxfer(0), m_numIrSessions(0), m_irSession(0),
        m_infraredService(0)
#endif
{
#ifdef QTOPIA_BLUETOOTH
    m_bluetoothService = new BluetoothPushingService(this);
#endif

    //just blow it all away and recreate the dir
    // Note that any files the user has not accepted from his incoming directory
    // (using the receive window widget) will be gone forever
    QProcess::execute("rm", QStringList() << "-rf" << OBEX_INCOMING_DIRECTORY);
    QProcess::execute("mkdir", QStringList() << "-p" << "-m" << "0755" << OBEX_INCOMING_DIRECTORY);

#ifdef QTOPIA_BLUETOOTH
    new ObexPushServiceProvider(this);
#endif

#ifdef QTOPIA_INFRARED
    QStringList irDevices = QIrLocalDevice::devices();
    qLog(Infrared) << "Got Infrared devices:" << irDevices;

    if (irDevices.size() > 0) {
        m_irxfer = new QIrObexServer("OBEX:IrXfer", QIr::OBEX | QIr::Telephony, this);
        connect(m_irxfer, SIGNAL(newConnection()), SLOT(newIrxferConnection()));
        m_irxfer->listen();

        //TODO: Register other IR services here
        m_irSession = new QCommDeviceSession(irDevices[0].toLatin1());
        m_infraredService = new InfraredBeamingService(this);
    }
    else {
        qWarning() << "No IRDA devices found!";
    }
#endif
}

/*!
    \internal
*/
ObexServiceManager::~ObexServiceManager()
{
#ifdef QTOPIA_INFRARED
    delete m_irxfer;
    delete m_irSession;
    delete m_infraredService;
#endif

#ifdef QTOPIA_BLUETOOTH
    delete m_bluetoothService;
#endif
}

/*!
    \internal

    Sets up a service connection.
*/
void ObexServiceManager::setupConnection(QObexPushService *opush)
{
    opush->setIncomingDirectory(OBEX_INCOMING_DIRECTORY);
    opush->setAutoDelete(true);

    connect(opush, SIGNAL(putRequest(const QString &, const QString &)),
            SLOT(putRequest(const QString &, const QString &)));
    connect(opush, SIGNAL(getRequest(const QString &, const QString &)),
            SLOT(getRequest(const QString &, const QString &)));
    connect(opush, SIGNAL(progress(qint64, qint64)), SLOT(progress(qint64, qint64)));
    connect(opush, SIGNAL(requestComplete(bool)), SLOT(requestComplete(bool)));
    connect(opush, SIGNAL(destroyed(QObject *)),
            SLOT(aboutToDelete()));

    m_map.insert(opush, 0);
}

/*!
    \internal

    Sets up a client connection.
*/
void ObexServiceManager::setupConnection(QObexPushClient *client,
                                         const QString &filename,
                                         const QString &mimetype)
{
    // We never do multiple put commands here, so this is safe
    connect(client, SIGNAL(progress(qint64, qint64)), SLOT(progress(qint64, qint64)));
    connect(client, SIGNAL(done(bool)), SLOT(requestComplete(bool)));
    connect(client, SIGNAL(destroyed(QObject *)),
            SLOT(aboutToDelete()));

    QMap<QObject *, int>::iterator i = m_map.insert(client, 0);

    i.value() = nextId();
    emit sendInitiated(i.value(), filename, mimetype);
}

#ifdef QTOPIA_INFRARED
/*!
    \internal
 */
void ObexServiceManager::newIrxferConnection()
{
    qLog(Infrared) << "New IrXfer Connection";
    if (!m_irxfer->hasPendingConnections())
        return;

    qLog(Infrared) << "Creating ObexPushService";
    CustomPushService *opush = new CustomPushService(m_irxfer->nextPendingConnection());

    setupConnection(opush);
    connect(opush, SIGNAL(destroyed(QObject *)),
            SLOT(irSessionEnded()));

    m_numIrSessions++;


    qLog(Infrared) << "Num IR Sessions: " << m_numIrSessions;

    if (m_numIrSessions == 1) {
        // We don't need to receive the notification of session start here
        // as that just notifies us that the device is up.  We would not be receiving
        // connections here if the device wasn't up though
        qLog(Infrared) << "ObexServiceManager starting IR Session";
        m_irSession->startSession();
    }
}
#endif

/*!
    \internal
 */
void ObexServiceManager::putRequest(const QString &filename, const QString &mimetype)
{
    qLog(Obex) << "ObexServiceManager Put Request";
    QObject *s = sender();
    if (!s)
        return;

    qLog(Obex) << "Sender is valid";

    QMap<QObject *, int>::iterator i = m_map.find(s);
    if (i == m_map.end())
        return;

    qLog(Obex) << "Sender in map";

    i.value() = nextId();

    emit receiveInitiated(i.value(), filename, mimetype);
}

/*!
    \internal
 */
void ObexServiceManager::getRequest(const QString &filename, const QString &mimetype)
{
    QObject *s = sender();
    if (!s)
        return;

    QMap<QObject *, int>::iterator i = m_map.find(s);
    if (i == m_map.end())
        return;

    i.value() = nextId();

    emit sendInitiated(i.value(), filename, mimetype);
}

/*!
    \internal
 */
void ObexServiceManager::requestComplete(bool error)
{
    QObject *s = sender();
    if (!s)
        return;

    QMap<QObject *, int>::iterator i = m_map.find(s);
    if (i == m_map.end())
        return;

    if (i.value() != 0) {
        emit completed(i.value(), error);
        i.value() = 0;
    }
}

/*!
    \internal
 */
void ObexServiceManager::progress(qint64 done, qint64 total)
{
    QObject *s = sender();
    if (!s)
        return;

    QMap<QObject *, int>::const_iterator i = m_map.find(s);
    if (i == m_map.end())
        return;

    emit progress(i.value(), done, total);
}

/*!
    \internal
 */
void ObexServiceManager::aboutToDelete()
{
    qLog(Obex) << "Service Object about to delete itself";

    QObject *s = sender();
    if (!s)
        return;

    QMap<QObject *, int>::iterator i = m_map.find(s);
    if (i == m_map.end())
        return;

    m_map.erase(i);
}

#ifdef QTOPIA_INFRARED
/*!
    \internal
 */
void ObexServiceManager::irSessionEnded()
{
    m_numIrSessions--;
    qLog(Infrared) << "Infrared Receive finished, NumIrSessions: " << m_numIrSessions;

    if (m_numIrSessions == 0) {
        qLog(Infrared) << "Ending Infrared session";
        m_irSession->endSession();
        qLog(Infrared) << "Session ended";
    }
}
#endif

QBasicAtomic ObexServiceManager::idCounter = Q_ATOMIC_INIT(1);

/*!
    \internal
 */
int ObexServiceManager::nextId()
{
    register int id;
    for (;;) {
        id = idCounter;
        if (idCounter.testAndSet(id, id + 1))
            break;
    }
    return id;
}

/*!
    \fn void ObexServiceManager::receiveInitiated(int id, const QString &filename, const QString &mime)

    This signal is sent whenever an object is being received.  The \a id parameter
    contains a unique id of the request.  The \a filename parameter contains the filename
    of the object being received.  This filename will be in an incoming directory (the inbox.)
    The \a mime contains the mimetype of the object being received.

    One can track the progress of the request by using the completed() and progress() signals.

    \sa completed(), progress()
*/

/*!
    \fn void ObexServiceManager::sendInitiated(int id, const QString &filename, const QString &mime)

    This signal is emitted whenever an object is being sent by the local device.  The \a id
    paramter contains a unique id of the request.  The \a filename parameter contains
    the filename of the object being send.  The \a mime contains the mimetype of the object
    being received.

    One can track the progress of the request by using the completed() and progress() signals.

    \sa completed(), progress()
*/

/*!
    \fn void ObexServiceManager::progress(int id, qint64 bytes, qint64 total)

    This signal is emitted to report progress of an operation.  The \a id parameter
    contains the unique id of the request.  The \a bytes paramter contains the number
    of bytes sent.  The \a total contains the total number of bytes.  If the total is
    not known, it will be set to 0.
*/

/*!
    \fn void ObexServiceManager::completed(int id, bool error)

    This signal is emitted to report the completion of an operation.  The \a id paramter
    contains the unique id of the request completed.  The \a error paramter
    is set to true if the operation finished due to an error, and false otherwise.
*/

#include "obexservicemanager.moc"
