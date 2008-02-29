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

#include <qbluetoothpasskeyrequest.h>

/*!
    \class QBluetoothPasskeyRequest
    \brief The QBluetoothPasskeyRequest class encapsulates a Bluetooth PIN request.

    The QBluetoothPasskeyRequest class encapsulates a passkey request received
    from the bluetooth system.  The request can be rejected, cancelled or
    accepted.  If it is accepted, a valid passkey must be provided.

    \ingroup qtopiabluetooth
    \sa QBluetoothPasskeyAgent
 */

/*!
    Constructs a QBluetoothPasskeyRequest.  The \a localDevice parameter
    specifies the local bluetooth adapter the request came in on.  The \a
    remoteDevice parameter specifies the remote device being paired to.

    By default, the request is set to be rejected.
*/
QBluetoothPasskeyRequest::QBluetoothPasskeyRequest(const QString &localDevice,
    const QBluetoothAddress &remoteDevice)
{
    m_rejected = true;
    m_localDevice = localDevice;
    m_remoteDevice = remoteDevice;
}

/*!
    Copy constructor.  Constructs a QBluetoothPasskeyRequest from \a other.
*/
QBluetoothPasskeyRequest::QBluetoothPasskeyRequest(const QBluetoothPasskeyRequest &other)
{
    operator=(other);
}

/*!
    Destructor.
*/
QBluetoothPasskeyRequest::~QBluetoothPasskeyRequest()
{

}

/*!
    Assignment operator.  Assigns the contents of \a other to the current
    passkey request.
*/
QBluetoothPasskeyRequest &QBluetoothPasskeyRequest::operator=(const QBluetoothPasskeyRequest &other)
{
    if (this == &other)
        return *this;

    m_localDevice = other.m_localDevice;
    m_remoteDevice = other.m_remoteDevice;
    m_passkey = other.m_passkey;
    m_rejected = other.m_rejected;

    return *this;
}

/*!
    Comparison operator.  Compares the contents of \a other to the current
    passkey request.  Returns true if the contents are equal.
*/
bool QBluetoothPasskeyRequest::operator==(const QBluetoothPasskeyRequest &other) const
{
    return ((m_localDevice == other.m_localDevice) &&
            (m_remoteDevice == other.m_remoteDevice) &&
            (m_passkey == other.m_passkey) &&
            (m_rejected == other.m_rejected));
}

/*!
    Comparison operator.  Compares the contents of \a other to the current
    passkey request. Returns true if the contents are not equal.
*/
bool QBluetoothPasskeyRequest::operator!=(const QBluetoothPasskeyRequest &other) const
{
    return !operator==(other);
}

/*!
    Returns the name of the local device adapter.  This can be passed in to the
    QBluetoothLocalDevice constructor.

    \sa QBluetoothLocalDevice
*/
const QString &QBluetoothPasskeyRequest::localDevice() const
{
    return m_localDevice;
}

/*!
    Returns the address of the remote device associated with this request.
*/
const QBluetoothAddress &QBluetoothPasskeyRequest::remoteDevice() const
{
    return m_remoteDevice;
}

/*!
    Sets the request to be rejected.  This usually means that the user
    has explicitly rejected the request.
*/
void QBluetoothPasskeyRequest::setRejected()
{
    m_rejected = true;
    m_passkey = QString();
}

/*!
    Returns true if the request has been rejected.
*/
bool QBluetoothPasskeyRequest::isRejected() const
{
    return m_rejected;
}

/*!
    Accepts the pairing request, and sets the passkey to \a passkey.
*/
void QBluetoothPasskeyRequest::setPasskey(const QString &passkey)
{
    m_passkey = passkey;
    m_rejected = false;
}

/*!
    Returns the passkey for the request.
*/
const QString &QBluetoothPasskeyRequest::passkey() const
{
    return m_passkey;
}
