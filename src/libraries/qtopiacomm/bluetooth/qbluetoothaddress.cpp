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

#include <qbluetoothaddress.h>
#include <qhash.h>

#include <string.h>
#include <stdio.h>

/*!
    \class QBluetoothAddress
    \brief The QBluetoothAddress class represents a bluetooth address.

The QBluetoothAddress class refers to a single Bluetooth address.
Each Bluetooth device can have one and only one address.  There are certain
special addresses defined.  Namely \bold{any}, \bold{local}, \bold{all}.

    \ingroup qtopiabluetooth
    \sa QBluetoothLocalDevice, QBluetoothRemoteDevice
 */

/*!
    Constructs a new invalid bluetooth address.
 */
QBluetoothAddress::QBluetoothAddress()
{
    m_valid = false;
    m_bdaddr = "00:00:00:00:00:00";
}

/*!
    Constructs a copy of a bluetooth address from \a other.
*/
QBluetoothAddress::QBluetoothAddress(const QBluetoothAddress &other)
{
    m_valid = other.m_valid;
    m_bdaddr = other.m_bdaddr;
}

/*!
    Constructs a bluetooth address based on string representation given by \a addr.
    The string should be in the format of XX:XX:XX:XX:XX:XX where XX represents
    a hexadecimal number.  If the string is in an invalid format, an invalid
    bluetooth address is constructed.
*/
QBluetoothAddress::QBluetoothAddress(const QString &addr)
{
    unsigned int baddr[6];

    QByteArray asciiArr = addr.toAscii();
    const char *buf = asciiArr.constData();

    if (sscanf(buf, "%x:%x:%x:%x:%x:%x",
        &baddr[0], &baddr[1], &baddr[2], &baddr[3], &baddr[4], &baddr[5]) == 6) {
        m_valid = true;
        m_bdaddr = addr;
    }
    else {
        m_valid = false;
        m_bdaddr = QBluetoothAddress::invalid.m_bdaddr;
    }
}

/*!
    Destroys a Bluetooth address.
*/
QBluetoothAddress::~QBluetoothAddress()
{
}

/*!
    Assigns the contents of the Bluetooth address \a other to the current address.
*/
QBluetoothAddress &QBluetoothAddress::operator=(const QBluetoothAddress &other)
{
    if (this == &other)
        return *this;

    m_valid = other.m_valid;
    m_bdaddr = other.m_bdaddr;

    return *this;
}

/*!
    Compares the current Bluetooth address against a Bluetooth address given by \a other.
*/
bool QBluetoothAddress::operator==(const QBluetoothAddress &other) const
{
    return m_bdaddr == other.m_bdaddr;
}

/*!
    Converts the Bluetooth address into a QString.  The format will be of the form
    XX:XX:XX:XX:XX:XX where XX is a hexadecimal number.  If the address is invalid, a
    null string is returned.
*/
QString QBluetoothAddress::toString() const
{
    if (!m_valid)
        return QString();

    return m_bdaddr;
}

/*!
    Returns whether the address is valid.
*/
bool QBluetoothAddress::isValid() const
{
    return m_valid;
}

/*!
    \variable QBluetoothAddress::invalid
    Invalid Bluetooth address.
*/
const QBluetoothAddress QBluetoothAddress::invalid = QBluetoothAddress();

/*!
    \variable QBluetoothAddress::any
    Bluetooth address that represents a special address \bold any
*/
const QBluetoothAddress QBluetoothAddress::any = QBluetoothAddress("00:00:00:00:00:00");

/*!
    \variable QBluetoothAddress::all
    Bluetooth address that represents a special address \bold all
 */
const QBluetoothAddress QBluetoothAddress::all = QBluetoothAddress("FF:FF:FF:FF:FF:FF");

/*!
    \variable QBluetoothAddress::local
    Bluetooth address that represents a special address \bold local
*/
const QBluetoothAddress QBluetoothAddress::local = QBluetoothAddress("00:00:00:FF:FF:FF");

/*!
    \fn bool QBluetoothAddress::operator!=(const QBluetoothAddress &other) const

    Compares the current address to \a other.
    Returns true if the addresses are not equal, false if they are.
*/

/*!
    \internal
    \fn void QBluetoothAddress::serialize(Stream &stream) const
 */
template <typename Stream> void QBluetoothAddress::serialize(Stream &stream) const
{
    stream << m_bdaddr;
    stream << m_valid;
}

/*!
    \internal
    \fn void QBluetoothAddress::deserialize(Stream &stream)
 */
template <typename Stream> void QBluetoothAddress::deserialize(Stream &stream)
{
    stream >> m_bdaddr;
    stream >> m_valid;
}

uint qHash(const QBluetoothAddress &addr)
{
    return qHash(addr.m_bdaddr);
}

Q_IMPLEMENT_USER_METATYPE(QBluetoothAddress)
