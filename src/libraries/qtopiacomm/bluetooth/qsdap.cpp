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

#include <qsdap.h>
#include <qsdpuuid.h>
#include <qsdpservice.h>
#include <qbluetoothlocaldevice.h>
#include <qbluetoothaddress.h>
#include <qbluetoothnamespace.h>
#include <qtopialog.h>
#include <qtopianamespace.h>

#include <qtopiacomm/private/qbluetoothnamespace_p.h>
#include <qtopiacomm/private/sdp_tokens_p.h>
#include <qtopiacomm/private/sdplexer_p.h>

#include <QList>
#include <QString>
#include <QProcess>
#include <QTextStream>
#include <QStack>
#include <QUrl>

/*!
    \class QSDAPSearchResult
    \brief The QSDAPSearchResult class encapsulates a result of a SDAP query.

The QSDAPSearchResult class encapsulates a result of a SDAP query, which is returned by the QSDAP object.
The query consists of a list of \c QSDPService objects.

    \ingroup qtopiabluetooth
    \sa QSDAP, QSDPService
 */

/*!
    Constructs an empty QSDAPSearchResult object.
*/
QSDAPSearchResult::QSDAPSearchResult()
{

}

/*!
    Construct a SDAPSearchResult, copying contents from \a other.
    */
QSDAPSearchResult::QSDAPSearchResult(const QSDAPSearchResult &other)
{
    (*this) = other; //use assignment operator
}

/*!
    Assign the contents of \a other to the current SDAPSearchResult.
  */
QSDAPSearchResult& QSDAPSearchResult::operator=(const QSDAPSearchResult &other)
{
    if (this == &other)
        return (*this);

    m_services.clear();
    m_services = other.m_services;
    m_error = other.m_error;
    return (*this);
}

/*!
    Adds a \a service to the search result.
*/
void QSDAPSearchResult::addService(const QSDPService &service)
{
    m_services.push_back(service);
}

/*!
    Returns whether an error occurred during the SDAP search.
*/
bool QSDAPSearchResult::errorOccurred() const
{
    if (m_error.isNull())
        return false;

    return true;
}

/*!
    Returns the error string, if any.  If no error occurred, a null
    string is returned.
*/
QString QSDAPSearchResult::error() const
{
    return m_error;
}

/*!
    Clears the error condition.
*/
void QSDAPSearchResult::clearError()
{
    setError(QString::null);
}

/*!
    Flags that an error occurred during a SDAP query and sets the error string to
    \a error.
*/
void QSDAPSearchResult::setError(const QString &error)
{
    m_error = error;
}

/*!
    Returns a list of services found during a SDAP query.
*/
const QList<QSDPService> &QSDAPSearchResult::services() const
{
    return m_services;
}

/*!
    Clears the service list.
*/
void QSDAPSearchResult::clear()
{
    m_services.clear();
}

struct QSDAP_Private {
    QSDAP_Private();
    ~QSDAP_Private();

    void reset();

    // QSDAP specific stuff
    QProcess m_process;
    QSDAPSearchResult m_result;
    bool m_finished;
    bool m_process_finished;
    sdpLexer *m_lexer;
    bool m_cancelled;

    // Parser specific data
    int m_indent_level;
    int m_prev_indent_level;
    QSDPService m_current_service;

    QStack<QVariant> m_stack;
    quint16 m_stack_attrid;
    enum State { EXPECT_INITIAL,
        EXPECT_ATTRID,
        EXPECT_BOOL,
        EXPECT_UINT8,
        EXPECT_INT8,
        EXPECT_UINT16,
        EXPECT_INT16,
        EXPECT_UINT32,
        EXPECT_INT32,
        EXPECT_UINT64,
        EXPECT_INT64,
        EXPECT_UINT128,
        EXPECT_INT128,
        EXPECT_UUID,
        EXPECT_TEXT,
        EXPECT_URL,
        EXPECT_DATA
    };
    State m_cur_state;
};

QSDAP_Private::QSDAP_Private()
{
    m_finished = true;
    m_process_finished = false;
    m_lexer = new sdpLexer(&m_process);

    m_indent_level = m_prev_indent_level = 0;
}

QSDAP_Private::~QSDAP_Private()
{
    delete m_lexer;
}

void QSDAP_Private::reset()
{
    m_result.clear();
    m_result.clearError();
    m_process_finished = false;

    m_cancelled = false;
    m_current_service = QSDPService();
    m_stack.clear();
    m_indent_level = m_prev_indent_level = 0;
    m_cur_state = EXPECT_INITIAL;
}

/*!
    \class QSDAP
    \brief The QSDAP class encapsulates operations performed by the SDAP profile.

The QSDAP class provides a way to use the facilities of the Bluetooth Service
Discovery Application Profile.  QSDAP allows the client to browse all services
or search for a specific service on a remote device.  QSDAP also allows the same
functionality to be performed for a local device.  The results are reported in the
\c QSDAPSearchResult object.

    \ingroup qtopiabluetooth
    \sa QSDAPSearchResult, QSDPService
 */

/*!
    Constructs a new QSDAP object.
*/
QSDAP::QSDAP()
{
    m_data = new QSDAP_Private;

    connect(&m_data->m_process, SIGNAL(readyReadStandardOutput()), SLOT(readServices()));
    connect(&m_data->m_process, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(processFinished()));
}

/*!
    Deconstructs a QSDAP object
*/
QSDAP::~QSDAP()
{
    if (m_data)
        delete m_data;
}

/*!
    Starts a new search query that searches for a specific Bluetooth service profile, e.g.
    OBEX Object PUSH.  The remote device to search is specified by the
    \a remote parameter.  The local device that will perform the search is specified by the
    \a local parameter.  The profile to search for is specified by the \a profile parameter.

    The method returns true if the request could be started successfully, and false
    otherwise (for instance, a request is already running).
*/
bool QSDAP::searchServices(const QBluetoothAddress &remote,
                           const QBluetoothLocalDevice &local,
                           QBluetooth::SDPProfile profile)
{
    if (!m_data->m_finished)
        return false;

    m_data->reset();

    QString sdptool = find_sdptool();
    qLog(Bluetooth) << "Starting sdptool in" << sdptool;

    m_data->m_process.start(sdptool, QStringList() <<
            "-i" << local.deviceName() << "search" << "--raw" <<
            "--bdaddr" << remote.toString() <<
            map_sdp_profile_to_bluez_name(profile));

    return true;
}

/*!
    Starts a new search query.  The remote device to search is
    specified by the \a remote parameter.  The local device
    that will perform the search is specified by the
    \a local parameter.  All services with attribute UUIDs that
    match a UUID provided by the \a uuid parameter will be returned.

    The method returns true if the request could be started
    successfully, and false otherwise (for instance, a request
    is already running).
 */
bool QSDAP::searchServices(const QBluetoothAddress &remote,
                           const QBluetoothLocalDevice &local,
                           const QSDPUUID &uuid)
{
    if (!m_data->m_finished)
        return false;

    m_data->reset();

    QString sdptool = find_sdptool();
    qLog(Bluetooth) << "Starting sdptool in" << sdptool;

    m_data->m_process.start(sdptool,
                            QStringList() << "-i" << local.deviceName() <<
                                    "search" << "--raw" <<
                                    "--bdaddr" << remote.toString() <<
                                    uuid.toString());

    return true;
}

/*!
     Starts a new search query that searches for a specific Bluetooth
    service profile, e.g. OBEX Object PUSH.  The local device to
    use for the search is specified by the \a local parameter.
    The profile to search for is specified by the \a profile parameter.

    The method returns true if the request could be started
    successfully, and false otherwise (for instance, a request
    is already running).
 */
bool QSDAP::searchLocalServices(const QBluetoothLocalDevice &local,
                                QBluetooth::SDPProfile profile)
{
    if (!m_data->m_finished)
        return false;

    m_data->reset();

    QString sdptool = find_sdptool();
    qLog(Bluetooth) << "Starting sdptool in" << sdptool;

    m_data->m_process.start(sdptool, QStringList() <<
            "-i" << local.deviceName() << "search" << "--raw" <<
            "--bdaddr" << "local" <<
            map_sdp_profile_to_bluez_name(profile));

    return true;
}

/*!
    Starts a new search query of the local device.  The local device
    to use for the search is specified by the \a local parameter.
    All services with attribute UUIDs that match a UUID provided by
    the \a uuid parameter will be returned.

    The method returns true if the request could be started
    successfully, and false otherwise (for instance, a request is
    already running).
 */
bool QSDAP::searchLocalServices(const QBluetoothLocalDevice &local,
                                const QSDPUUID &uuid)
{
    if (!m_data->m_finished)
        return false;

    m_data->reset();

    QString sdptool = find_sdptool();
    qLog(Bluetooth) << "Starting sdptool in" << sdptool;

    m_data->m_process.start(sdptool, QStringList() <<
            "-i" << local.deviceName() << "search" << "--raw" <<
                    "--bdaddr" << "local" <<
                    uuid.toString());

    return true;
}

/*!
    Starts a new browse query of the remote device.  This query
    will return all services which are listed in the public browse
    group of the device, and any subgroups which are children of the
    public browse group.  The remote device to search is specified
    by the \a remote parameter.  The local device that
    will perform the  search is specified by the \a local parameter.

    The method returns true if the request could be started
    successfully, and false otherwise (for instance, a request is
    already running).

    Please note, that if you require all services, which might not be
    part of the public browse group, you can try to perform a search
    using the L2CAP UUID (0x0100).

    \code
        QSDAP qsdap;
        qsdap.searchServices(0x0100);
    \endcode
*/
bool QSDAP::browseServices(const QBluetoothAddress &remote,
                    const QBluetoothLocalDevice &local)
{
    if (!m_data->m_finished)
        return false;

    m_data->reset();

    QString sdptool = find_sdptool();
    qLog(Bluetooth) << "Starting sdptool in" << sdptool;

    m_data->m_process.start(sdptool, QStringList() <<
            "-i" << local.deviceName() << "browse" << "--raw" <<
                    remote.toString());

    return true;
}

/*!
    Starts a new browse query of the local device.  This query will
    return all services which are listed in the public browse group
    of the device, and any subgroups which are children of the
    public browse group.  The local device to search is specified
    by the \a local parameter.

    The method returns true if the request could be started
    successfully, and false otherwise (for instance, a request
    is already running).

    Please note, that if you require all services, which might not be
    part of the public browse group, you can try to perform a search
    using the L2CAP UUID (0x0100).

    \code
        QSDAP qsdap;
        qsdap.searchLocalServices(0x0100);
    \endcode
 */
bool QSDAP::browseLocalServices(const QBluetoothLocalDevice &local)
{
    if (!m_data->m_finished)
        return false;

    m_data->reset();

    QString sdptool = find_sdptool();
    qLog(Bluetooth) << "Starting sdptool in" << sdptool;

    m_data->m_process.start(sdptool, QStringList() <<
            "-i" << local.deviceName() << "browse" << "--raw" <<
                    "local");

    return true;
}

static void addAttribute(QSDAP_Private *data, const QVariant &var)
{
    // If stack is empty, then presumably we're adding a top-level attribute
    if (data->m_stack.empty()) {
        data->m_current_service.addAttribute(data->m_stack_attrid, var);
        return;
    }

    // Ugliness alert!!
    if (data->m_stack.top().canConvert<QSDPSequence>()) {
        QSDPSequence *seq =
                static_cast<QSDPSequence *>(data->m_stack.top().data());
        seq->push_back(var);
    }
    else if (data->m_stack.top().canConvert<QSDPAlternative>()) {
        QSDPAlternative *alt =
                static_cast<QSDPAlternative *>(data->m_stack.top().data());
        alt->push_back(var);
    }
    else {
        qLog(Bluetooth) << "Unknown type in the QVariant, should be either a sequence or an alternative";
    }

}

static void pushStack(QSDAP_Private *data, const QVariant &var)
{
    data->m_prev_indent_level = data->m_indent_level;
    data->m_stack.push(var);
}

static void popStack(QSDAP_Private *data)
{
    // Assume indentation of 8 is a single level
    while (!data->m_stack.empty() && (data->m_indent_level <= data->m_prev_indent_level)) {
        QVariant popped = data->m_stack.pop();

        if (data->m_stack.count()) {
            // Need to add the popped attribute to the top of the stack
            // Check if this is a ALT or a SEQ or ATTR(represented by quint16)
            addAttribute(data, popped);
        }
        else {
            data->m_current_service.addAttribute(data->m_stack_attrid, popped);
        }

        data->m_prev_indent_level -= 8;
    }
}

static void parseInitialState(QSDAP_Private *data, int token)
{
    switch (token) {
        case SDP_NEWLINE:
            data->m_indent_level = 0;
            data->m_cur_state = QSDAP_Private::EXPECT_INITIAL;
            break;
        case SDP_SPACE:
            data->m_indent_level++;
            break;
        case SDP_TAB:
            data->m_indent_level+=8;
            break;
        case SDP_SEQ:
        {
            // Two cases here, if we're at indent level 0, then it is not really
            // a sequence, but a new service record
            if (data->m_indent_level == 0) {
                    // Previous indent level should be 0 if this is the first service
                    // encountered, otherwise it should be > 0
                if (data->m_prev_indent_level) {
                    popStack(data);

                    data->m_result.addService(data->m_current_service);
                    data->m_current_service = QSDPService();
                }
            }
            else {
                popStack(data);
                QSDPSequence seq;
                QVariant var = QVariant::fromValue(seq);
                pushStack(data, var);
            }
            break;
        }
        case SDP_ALT:
        {
            popStack(data);
            QSDPAlternative alt;
            QVariant var(alt);
            pushStack(data, var);
            break;
        }
        case SDP_ATTR:
            popStack(data);
            data->m_cur_state = QSDAP_Private::EXPECT_ATTRID;
            break;
        case SDP_NIL:
        {
            qLog(Bluetooth) << "Got a SDP_NIL value";
            QVariant nil;
            pushStack(data, nil);
            popStack(data);
            break;
        }
        case SDP_BOOL:
            data->m_cur_state = QSDAP_Private::EXPECT_BOOL;
            popStack(data);
            break;
        case SDP_UINT8:
            data->m_cur_state = QSDAP_Private::EXPECT_UINT8;
            popStack(data);
            break;
        case SDP_INT8:
            data->m_cur_state = QSDAP_Private::EXPECT_INT8;
            popStack(data);
            break;
        case SDP_UINT16:
            data->m_cur_state = QSDAP_Private::EXPECT_UINT16;
            popStack(data);
            break;
        case SDP_INT16:
            data->m_cur_state = QSDAP_Private::EXPECT_INT16;
            popStack(data);
            break;
        case SDP_UINT32:
            data->m_cur_state = QSDAP_Private::EXPECT_UINT32;
            popStack(data);
            break;
        case SDP_INT32:
            data->m_cur_state = QSDAP_Private::EXPECT_INT32;
            popStack(data);
            break;
        case SDP_UINT64:
            data->m_cur_state = QSDAP_Private::EXPECT_UINT64;
            popStack(data);
            break;
        case SDP_INT64:
            data->m_cur_state = QSDAP_Private::EXPECT_INT64;
            popStack(data);
            break;
        case SDP_UINT128:
            data->m_cur_state = QSDAP_Private::EXPECT_UINT128;
            popStack(data);
            break;
        case SDP_INT128:
            data->m_cur_state = QSDAP_Private::EXPECT_INT128;
            popStack(data);
            break;
        case SDP_UUID16:
        case SDP_UUID32:
        case SDP_UUID128:
            data->m_cur_state = QSDAP_Private::EXPECT_UUID;
            popStack(data);
            break;
        case SDP_TEXT:
            data->m_cur_state = QSDAP_Private::EXPECT_TEXT;
            popStack(data);
            break;
        case SDP_URL:
            data->m_cur_state = QSDAP_Private::EXPECT_URL;
            popStack(data);
            break;
        case SDP_DATA:
            data->m_cur_state = QSDAP_Private::EXPECT_DATA;
            popStack(data);
            break;
        default:
            qLog(Bluetooth) << "Unexpected token encountered: " << token;
            break;
    };
}

#define EXPECT_TOKEN(token, expected)               \
    do {                                            \
        if (token != expected) {                    \
            return;                                 \
        }                                           \
       } while (0)

static void parseOtherStates(QSDAP_Private *data, int token)
{
    if (token == SDP_NEWLINE) {
        data->m_cur_state = QSDAP_Private::EXPECT_INITIAL;
        data->m_indent_level = 0;
        return;
    }

    switch (data->m_cur_state) {
        case QSDAP_Private::EXPECT_ATTRID:
        {
            EXPECT_TOKEN(token, SDP_HEX16_VALUE);

            data->m_stack_attrid = data->m_lexer->text().toULong(0, 16);
            break;
        }
        case QSDAP_Private::EXPECT_BOOL:
        {
            EXPECT_TOKEN(token, SDP_BOOL_VALUE);

            if (data->m_lexer->text() == "False") {
                addAttribute(data, QVariant(false));
            }
            else {
                addAttribute(data, QVariant(true));
            }
            break;
        }
        case QSDAP_Private::EXPECT_UINT8:
        {
            EXPECT_TOKEN(token, SDP_HEX8_VALUE);

            QVariant var =
                    QVariant::fromValue<quint8>(data->m_lexer->text().toULong(0, 16));
            addAttribute(data, var);
            break;
        }
        case QSDAP_Private::EXPECT_INT8:
        {
            EXPECT_TOKEN(token, SDP_HEX8_VALUE);

            QVariant var =
                    QVariant::fromValue<qint8>(data->m_lexer->text().toLong(0, 16));
            addAttribute(data, var);
            break;
        }
        case QSDAP_Private::EXPECT_UINT16:
        {
            EXPECT_TOKEN(token, SDP_HEX16_VALUE);

            QVariant var =
                    QVariant::fromValue<quint16>(data->m_lexer->text().toULong(0, 16));
            addAttribute(data, var);
            break;
        }
        case QSDAP_Private::EXPECT_INT16:
        {
            EXPECT_TOKEN(token, SDP_HEX16_VALUE);

            QVariant var =
                    QVariant::fromValue<qint16>(data->m_lexer->text().toLong(0, 16));
            addAttribute(data, var);
            break;
        }
        case QSDAP_Private::EXPECT_UINT32:
        {
            EXPECT_TOKEN(token, SDP_HEX32_VALUE);

            QVariant var =
                    QVariant::fromValue<quint32>(data->m_lexer->text().toULong(0, 16));
            addAttribute(data, var);
            break;
        }
        case QSDAP_Private::EXPECT_INT32:
        {
            EXPECT_TOKEN(token, SDP_HEX32_VALUE);

            QVariant var =
                    QVariant::fromValue<qint32>(data->m_lexer->text().toLong(0, 16));
            addAttribute(data, var);
            break;
        }
        case QSDAP_Private::EXPECT_UINT64:
        {
            EXPECT_TOKEN(token, SDP_HEX64_VALUE);

            QVariant var =
                    QVariant::fromValue<quint64>(data->m_lexer->text().toULongLong(0, 16));
            addAttribute(data, var);
            break;
        }
        case QSDAP_Private::EXPECT_INT64:
        {
            EXPECT_TOKEN(token, SDP_HEX64_VALUE);
            QVariant var =
                    QVariant::fromValue<qint64>(data->m_lexer->text().toLongLong(0, 16));
            addAttribute(data, var);
            break;
        }
        case QSDAP_Private::EXPECT_UINT128:
        {
            EXPECT_TOKEN(token, SDP_HEX128_VALUE);

            //TODO: sdptool does not output this at the moment
            QVariant var = QVariant::fromValue<quint128>(quint128());
            addAttribute(data, var);
            break;
        }
        case QSDAP_Private::EXPECT_INT128:
        {
            EXPECT_TOKEN(token, SDP_HEX128_VALUE);
            //TODO: sdptool does not output this at the moment
            QVariant var = QVariant::fromValue<qint128>(qint128());
            addAttribute(data, var);
            break;
        }
        case QSDAP_Private::EXPECT_UUID:
        {
            if ( (token == SDP_HEX16_VALUE) ||
                 (token == SDP_HEX32_VALUE) ||
                 (token == SDP_UUID128_VALUE) ) {
                QVariant var = QVariant::fromValue(QSDPUUID(data->m_lexer->text()));
                addAttribute(data, var);
            }
            else {
                qLog(Bluetooth) << "Expected SDP_HEX16_VALUE, SDP_HEX32_VALUE or SDP_UUID128_VALUE token";
            }
            break;
        }
        case QSDAP_Private::EXPECT_TEXT:
        {
            EXPECT_TOKEN(token, SDP_STRING_VALUE);

            QVariant var = QVariant::fromValue(QString(data->m_lexer->text()));
            addAttribute(data, var);
            break;
        }
        case QSDAP_Private::EXPECT_URL:
        {
            EXPECT_TOKEN(token, SDP_STRING_VALUE);

            QVariant var = QVariant::fromValue(QUrl(data->m_lexer->text()));
            addAttribute(data, var);
            break;
        }
        case QSDAP_Private::EXPECT_DATA:
        {
            EXPECT_TOKEN(token, SDP_BINARY_DATA_VALUE);

            QByteArray arr;
            QByteArray in = data->m_lexer->text().toAscii();
            char ch;
            unsigned int f;
            const char *ptr = in.constData();
            while (sscanf(ptr, "%2x", &f)) {
                ch = f;
                arr.append(ch);
                ptr += 2;
            }

            QVariant var = QVariant::fromValue(arr);
            addAttribute(data, var);
            break;
        }
        default:
            qLog(Bluetooth) << "Unexpected state!!";
    };
}

static void parseInput(QSDAP_Private *data)
{
    int token;

    while ((token = data->m_lexer->token())) {
        if (data->m_cur_state == QSDAP_Private::EXPECT_INITIAL)
            parseInitialState(data, token);
        else
            parseOtherStates(data, token);
    }
}

/*!
    \internal

    This function is the driver function for parsing output from sdptool
*/
void QSDAP::readServices()
{
    qLog(Bluetooth) << "QSDAP::readServices() called...";
    if (m_data->m_cancelled)
        return;

    qLog(Bluetooth) << "QSDAP::readServices() checking if process finished...";
    // If process exited with an error code, don't do anything
    // signal will be emitted appropriately in processFinished()
    if (m_data->m_process_finished) {
        if ((m_data->m_process.exitStatus() != QProcess::QProcess::NormalExit) ||
                (m_data->m_process.exitCode() != 0))
            return;
    }

    qLog(Bluetooth) << "calling parseInput";
    m_data->m_process.setReadChannel(QProcess::StandardOutput);
    parseInput(m_data);

    //Check to make sure we've read all data, if we have, send notifications
    if (m_data->m_process_finished) {
        popStack(m_data);
        m_data->m_result.addService(m_data->m_current_service);
        m_data->m_finished = true;
        qLog(Bluetooth) << "QSDAP::readServices emitting searchComplete";
        emit searchComplete(m_data->m_result);
    }
}

/*!
    \internal

    This function is called whenever sdptool process finished
*/
void QSDAP::processFinished()
{
    qLog(Bluetooth) << "sdptool process has finished...";

    m_data->m_process_finished = true;

    if (m_data->m_cancelled)
        return;

    if ((m_data->m_process.exitStatus() != QProcess::QProcess::NormalExit) ||
         (m_data->m_process.exitCode() != 0)) {
        QString error = QString(m_data->m_process.readAllStandardError());
        if (error.isNull())
            error = "";
        m_data->m_result.setError(error);
        qLog(Bluetooth) << "QSDAP::processFinished emitting searchComplete with error";
        emit searchComplete(m_data->m_result);
        return;
    }

    //Check to make sure we've read all data, if we have, send notifications
    if (m_data->m_process.atEnd()) {
        popStack(m_data);
        m_data->m_result.addService(m_data->m_current_service);
        m_data->m_finished = true;
        qLog(Bluetooth) << "QSDAP::processFinished emitting searchComplete";
        emit searchComplete(m_data->m_result);
    }
}

/*!
    Cancels the SDAP query.
*/
void QSDAP::cancelSearch()
{
    m_data->m_cancelled = true;
    m_data->m_finished = true;
    m_data->m_process.kill();
}

/*!
    \fn void QSDAP::searchComplete(const QSDAPSearchResult &result)

    This signal is emitted when an SDAP result has completed.  The result of the scan
    is reported by the \a result parameter.

    \sa QSDAPSearchResult
 */
