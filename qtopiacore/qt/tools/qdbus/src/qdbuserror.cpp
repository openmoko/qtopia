/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qdbuserror.h"

#include <qdebug.h>
#include <qvarlengtharray.h>

#include <dbus/dbus.h>
#include "qdbusmessage.h"
#include "qdbusmessage_p.h"

struct ErrorMessageMapping
{
    ErrorMessageMapping();
    QVarLengthArray<const char*, QDBusError::LastErrorType> messages;

    inline const char *get(QDBusError::ErrorType code) const
    {
        if (code <= QDBusError::Other || code > QDBusError::LastErrorType)
            return messages[int(QDBusError::Other) - 1];
        return messages[int(code) - 1];
    }

    inline QDBusError::ErrorType get(const char *name) const
    {
        if (!name || !*name)
            return QDBusError::NoError;
        for (int i = QDBusError::Other; i <= QDBusError::LastErrorType; ++i)
            if (strcmp(name, messages[i - 1]) == 0)
                return QDBusError::ErrorType(i);
        return QDBusError::Other;
    }
};

static const char errorMessages_string[] =
    // in the same order as KnownErrors!
    "other\0"                           // Other -- shouldn't happen
    DBUS_ERROR_FAILED "\0"              // Failed
    DBUS_ERROR_NO_MEMORY "\0"           // NoMemory
    DBUS_ERROR_SERVICE_UNKNOWN "\0"     // ServiceUnknown
    DBUS_ERROR_NO_REPLY "\0"            // NoReply
    DBUS_ERROR_BAD_ADDRESS "\0"         // BadAddress
    DBUS_ERROR_NOT_SUPPORTED "\0"       // NotSupported
    DBUS_ERROR_LIMITS_EXCEEDED "\0"     // LimitsExceeded
    DBUS_ERROR_ACCESS_DENIED  "\0"      // AccessDenied
    DBUS_ERROR_NO_SERVER "\0"           // NoServer
    DBUS_ERROR_TIMEOUT "\0"             // Timeout
    DBUS_ERROR_NO_NETWORK "\0"          // NoNetwork
    DBUS_ERROR_ADDRESS_IN_USE "\0"      // AddressInUse
    DBUS_ERROR_DISCONNECTED "\0"        // Disconnected
    DBUS_ERROR_INVALID_ARGS "\0"        // InvalidArgs
    DBUS_ERROR_UNKNOWN_METHOD "\0"      // UnknownMethod
    DBUS_ERROR_TIMED_OUT "\0"           // TimedOut
    DBUS_ERROR_INVALID_SIGNATURE "\0"   // InvalidSignature
    "com.trolltech.QtDBus.Error.UnknownInterface\0" // UnknownInterface
    "com.trolltech.QtDBus.Error.InternalError\0" // InternalError
    "\0";

ErrorMessageMapping::ErrorMessageMapping()
    : messages(int(QDBusError::LastErrorType))
{
    // create the list:
    const char *p = errorMessages_string;
    int i = 0;
    while (*p) {
        messages[i] = p;
        p += strlen(p) + 1;
        ++i;
    }
}

Q_GLOBAL_STATIC(ErrorMessageMapping, errorMessages)

/*!
    \class QDBusError
    \inmodule QtDBus
    \since 4.2

    \brief The QDBusError class represents an error received from the
    D-Bus bus or from remote applications found in the bus.

    When dealing with the D-Bus bus service or with remote
    applications over D-Bus, a number of error conditions can
    happen. This error conditions are sometimes signalled by a
    returned error value or by a QDBusError.

    C++ and Java exceptions are a valid analogy for D-Bus errors:
    instead of returning normally with a return value, remote
    applications and the bus may decide to throw an error
    condition. However, the QtDBus implementation does not use the C++
    exception-throwing mechanism, so you will receive QDBusErrors in
    the return reply (see QDBusReply::error()).

    QDBusError objects are used to inspect the error name and message
    as received from the bus and remote applications. You should not
    create such objects yourself to signal error conditions when
    called from D-Bus: instead, use QDBusMessage::createError() and
    QDBusConnection::send().

    \sa QDBusConnection::send(), QDBusMessage, QDBusReply
*/

/*!
    \enum QDBusError::ErrorType

    In order to facilitate verification of the most common D-Bus errors generated by the D-Bus
    implementation and by the bus daemon itself, QDBusError can be compared to a set of pre-defined
    values:

    \value NoError              QDBusError is invalid (i.e., the call succeeded)
    \value Other                QDBusError contains an error that is one of the well-known ones
    \value Failed               The call failed (\c org.freedesktop.DBus.Error.Failed)
    \value NoMemory             Out of memory (\c org.freedesktop.DBus.Error.NoMemory)
    \value ServiceUnknown       The called service is not known
                                (\c org.freedesktop.DBus.Error.ServiceUnknown)
    \value NoReply              The called method did not reply within the specified timeout
                                (\c org.freedesktop.DBus.Error.NoReply)
    \value BadAddress           The address given is not valid
                                (\c org.freedesktop.DBus.Error.BadAddress)
    \value NotSupported         The call/operation is not supported
                                (\c org.freedesktop.DBus.Error.NotSupported)
    \value LimitsExceeded       The limits allocated to this process/call/connection exceeded the
                                pre-defined values (\c org.freedesktop.DBus.Error.LimitsExceeded)
    \value AccessDenied         The call/operation tried to access a resource it isn't allowed to
                                (\c org.freedesktop.DBus.Error.AccessDenied)
    \value NoServer             \e {Documentation doesn't say what this is for}
                                (\c org.freedesktop.DBus.Error.NoServer)
    \value Timeout              \e {Documentation doesn't say what this is for or how it's used}
                                (\c org.freedesktop.DBus.Error.Timeout)
    \value NoNetwork            \e {Documentation doesn't say what this is for}
                                (\c org.freedesktop.DBus.Error.NoNetwork)
    \value AddressInUse         QDBusServer tried to bind to an address that is already in use
                                (\c org.freedesktop.DBus.Error.AddressInUse)
    \value Disconnected         The call/process/message was sent after QDBusConnection disconnected
                                (\c org.freedesktop.DBus.Error.Disconnected)
    \value InvalidArgs          The arguments passed to this call/operation are not valid
                                (\c org.freedesktop.DBus.Error.InvalidArgs)
    \value UnknownMethod        The method called was not found in this object/interface with the
                                given parameters (\c org.freedesktop.DBus.Error.UnknownMethod)
    \value TimedOut             \e {Documentation doesn't say...}
                                (\c org.freedesktop.DBus.Error.TimedOut)
    \value InvalidSignature     The type signature is not valid or compatible
                                (\c org.freedesktop.DBus.Error.InvalidSignature)
    \value UnknownInterface     The interface is not known
    \value InternalError        An internal error occurred
                                (\c com.trolltech.QtDBus.Error.InternalError)

*/

/*!
    \internal
    Constructs a QDBusError from a DBusError structure.
*/
QDBusError::QDBusError(const DBusError *error)
    : code(NoError)
{
    if (!error || !dbus_error_is_set(error))
        return;

    code = errorMessages()->get(error->name);
    msg = QString::fromUtf8(error->message);
    nm = QString::fromUtf8(error->name);
}

/*!
    \internal
    Constructs a QDBusError from a QDBusMessage.
*/
QDBusError::QDBusError(const QDBusMessage &qdmsg)
    : code(NoError)
{
    if (qdmsg.type() != QDBusMessage::ErrorMessage)
        return;

    if (qdmsg.arguments().count())
        msg = qdmsg.arguments().at(0).toString();

    code = errorMessages()->get(qdmsg.errorName().toUtf8().constData());
    nm = qdmsg.errorName();
}

/*!
    \internal
    Constructs a QDBusError from a well-known error code
*/
QDBusError::QDBusError(ErrorType error, const QString &mess)
    : code(error)
{
    nm = QLatin1String(errorMessages()->get(error));
    msg = mess;
}

/*!
    \internal
    Constructs a QDBusError from another QDBusError object
*/
QDBusError::QDBusError(const QDBusError &other)
        : code(other.code), msg(other.msg), nm(other.nm)
{
}

/*!
  \internal
  Assignment operator
*/

QDBusError &QDBusError::operator=(const QDBusError &other)
{
    code = other.code;
    msg = other.msg;
    nm = other.nm;
    return *this;
}

/*!
    Returns this error's ErrorType.

    \sa ErrorType
*/

QDBusError::ErrorType QDBusError::type() const
{
    return code;
}

/*!
    Returns this error's name. Error names are similar to D-Bus Interface names, like
    \c org.freedesktop.DBus.InvalidArgs.

    \sa type()
*/

QString QDBusError::name() const
{
    return nm;
}

/*!
    Returns the message that the callee associated with this error. Error messages are
    implementation defined and usually contain a human-readable error code, though this does not
    mean it is suitable for your end-users.
*/

QString QDBusError::message() const
{
    return msg;
}

/*!
    Returns true if this is a valid error condition (i.e., if there was an error),
    otherwise false.
*/

bool QDBusError::isValid() const
{
    return (code != NoError);
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const QDBusError &msg)
{
    dbg.nospace() << "QDBusError(" << msg.name() << ", " << msg.message() << ")";
    return dbg.space();
}
#endif


